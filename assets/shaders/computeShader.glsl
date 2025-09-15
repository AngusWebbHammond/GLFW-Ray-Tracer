#version 450 core

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout (binding = 0, rgba32f) uniform image2D img_output;

struct Material {
    vec4 materialColour; // xyz = color, w = reflectivity
    vec4 emmissiveColor; // xyz = emission, w = intensity
};

struct Sphere {
    vec4 centre; // xyz = position, w = radius
    Material material;
};

layout(std430, binding = 1) buffer Spheres {
    Sphere spheres[];
};

layout(std140, binding = 2) uniform Params { 
    vec4 info; // x = sphere count, y = frame count, z = accumulation count, w = isAccumulating
    vec4 backgroundColourAndNumBounces; // xyz = background colour, w = number of bounces
    float currentTime;
};

struct Camera {
    vec3 position;
    vec3 forward;
    vec3 up;
    vec3 right;
    float fov;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct RayHit {
    int sphereIndex;
    float t;
    vec3 lightAccumulation;
    vec3 colourAccumulation;
};

vec3 rayDirection(Camera camera, vec2 uv) {
    vec3 dir = normalize(
        camera.forward +
        (uv.x - 0.5) * camera.right * tan(radians(camera.fov) * 0.5) * 2.0 +
        (uv.y - 0.5) * camera.up * tan(radians(camera.fov) * 0.5) * 2.0
    );
    return dir;
}

bool isIntersectSphere(Ray ray, Sphere sphere, inout RayHit rayHit) {
    vec3 oc = ray.origin - sphere.centre.xyz;
    float bTerm = dot(oc, ray.direction);
    float cTerm = dot(oc, oc) - sphere.centre.w * sphere.centre.w;

    float discriminant = bTerm * bTerm - cTerm;
    if (discriminant < 0.0) return false;

    float t0 = -bTerm - sqrt(discriminant);
    float t1 = -bTerm + sqrt(discriminant);

    if (t0 > 0.001) rayHit.t = t0;
    else if (t1 > 0.001) rayHit.t = t1;
    else return false;

    return true;
}


// Thanks to https://amindforeverprogramming.blogspot.com/2013/07/random-floats-in-glsl-330.html
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

float random( float f ) {
    uint x = floatBitsToUint(f);
    x ^= x << 13u;
    x ^= x >> 17u;
    x ^= x << 5u;
    x = hash(x);
    return x * (1.0 / 4294967295.0);
}

// Uniform random point on unit sphere
vec3 getRandomOnUnitSphere(float seed) {
    float z = random(seed) * 2.0 - 1.0;
    float t = random(seed + 1.0) * 6.28318530718;
    float r = sqrt(1.0 - z*z);
    return vec3(r * cos(t), r * sin(t), z);
}

// Hemisphere biased along normal
vec3 getRandomOnUnitHemisphere(vec3 normal, float seed) {
    vec3 dir = getRandomOnUnitSphere(seed);
    if (dot(dir, normal) < 0.0) dir = -dir;
    return dir;
}

// Debug Random Noise
// void main() {
//     ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
//     ivec2 size = imageSize(img_output);
//     if (pixel.x >= size.x || pixel.y >= size.y) return;

//     // Use pixel + frame count as seed
//     float seed = float(pixel.x + pixel.y * size.x) + info.y * 223498345.234;

//     // Generate 3 random floats for RGB
//     float r = random(seed);
//     float g = random(seed + 1.0);
//     float b = random(seed + 2.0);

//     // Store directly as color
//     imageStore(img_output, pixel, vec4(r, g, b, 1.0));
// }

void main() {
    Camera camera;
    camera.position = vec3(0.0, 0.0, 10.0f);
    camera.forward = vec3(0.0, 0.0, -1.0);
    camera.up = vec3(0.0, 1.0, 0.0);
    camera.right = cross(camera.forward, camera.up);
    camera.fov = 45.0;

    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(img_output);
    if (pixel.x >= size.x || pixel.y >= size.y) return;

    

    vec2 uv = vec2(pixel) / vec2(size);
    Ray ray;
    ray.origin = camera.position;
    ray.direction = rayDirection(camera, uv);

    vec3 accumulatedColor = vec3(0.0);
    float accumulatedWeight = 1.0f;

    RayHit rayHit;
    rayHit.lightAccumulation = vec3(0.0);
    rayHit.colourAccumulation = vec3(1.0);

    for (int bounce = 0; bounce < backgroundColourAndNumBounces.w; bounce++) {
        float closestT = 1e20;
        rayHit.t = 1e20;
        rayHit.sphereIndex = -1;
        
        // Find closest intersection
        for (int i = 0; i < int(info.x); i++) {
            RayHit tempHit = rayHit;
            if (isIntersectSphere(ray, spheres[i], tempHit)) {
                if (tempHit.t < closestT) {
                    closestT = tempHit.t;
                    rayHit = tempHit;
                    rayHit.sphereIndex = i;
                }
            }
        }

        if (rayHit.sphereIndex < 0) {
            // No hit, add background and terminate
            accumulatedColor += vec3(backgroundColourAndNumBounces.xyz) * rayHit.colourAccumulation * accumulatedWeight;
            break;
        }

        Sphere hitSphere = spheres[rayHit.sphereIndex];
        vec3 hitPoint = ray.origin + ray.direction * rayHit.t;
        vec3 normal = normalize(hitPoint - hitSphere.centre.xyz);

        // Accumulate emission + material color
        
        rayHit.lightAccumulation += hitSphere.material.emmissiveColor.xyz * hitSphere.material.emmissiveColor.w * accumulatedWeight;
        accumulatedColor += (rayHit.colourAccumulation) * rayHit.lightAccumulation;
        rayHit.colourAccumulation *= hitSphere.material.materialColour.xyz;
        
        // Compute new ray direction (diffuse + specular)
        float reflectivity = hitSphere.material.materialColour.w;
        float seed = float(bounce) * 12.9898 + float(pixel.x + pixel.y * size.x) * 78.233 + currentTime*info.y;
        vec3 randomDir = getRandomOnUnitHemisphere(normal, seed);

        ray.origin = hitPoint + normal * 1e-4;
        ray.direction = normalize((1.0 - reflectivity) * normalize(normal + randomDir) + reflectivity * reflect(ray.direction, normal));

        // Reduce weight for next bounce
        accumulatedWeight *= 0.75;
    }

    if (info.w > 0.5) {
        vec4 prev = imageLoad(img_output, pixel);
        // Accumulate with previous frame
        accumulatedColor = (prev.xyz * info.z + accumulatedColor) / (info.z + 1.0);
        imageStore(img_output, pixel, vec4(accumulatedColor, 1.0));
    }

    else {
        // No accumulation
        imageStore(img_output, pixel, vec4(accumulatedColor, 1.0));
    }
}
