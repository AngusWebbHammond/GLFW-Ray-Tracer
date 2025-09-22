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

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 normal;
    Material material;
};

layout(std430, binding = 3) buffer Triangles {
    Triangle triangles[];
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
    int triangleIndex;
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

bool isIntersectTriangle(Ray ray, Triangle triangle, inout RayHit rayHit) {
    float denom = dot(ray.direction, triangle.normal);
    if (abs(denom) < 1e-6) return false; // parallel

    float t = dot(triangle.v0 - ray.origin, triangle.normal) / denom;
    if (t < 0.001) return false; // behind ray

    vec3 hitPoint = ray.origin + ray.direction * t;

    vec3 e0 = triangle.v1 - triangle.v0;
    vec3 e1 = triangle.v2 - triangle.v0;
    vec3 p  = hitPoint - triangle.v0;

    float dot00 = dot(e0, e0);
    float dot01 = dot(e0, e1);
    float dot02 = dot(e0, p);
    float dot11 = dot(e1, e1);
    float dot12 = dot(e1, p);

    float invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    if (u < 0.0 || v < 0.0 || u + v > 1.0) return false;

    rayHit.t = t;
    return true;
}

void main() {
    Camera camera;
    camera.position = vec3(0.0, 0.0, 10.0);
    camera.fov = 45.0;

    float yaw   = -90* 3.14/180;   // rotate around Y
    float pitch = 0.0* 3.14/180;   // look up/down

    camera.forward = normalize(vec3(
        cos(pitch) * cos(yaw),
        sin(pitch),
        cos(pitch) * sin(yaw)
    ));

    vec3 worldUp = vec3(0.0, 1.0, 0.0);
    camera.right   = normalize(cross(camera.forward, worldUp));
    camera.up      = normalize(cross(camera.right, camera.forward));

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
        rayHit.triangleIndex = -1;
        
        // Find closest intersection
        
        for (int i = 0; i < triangles.length(); i++) {
            RayHit tempHit = rayHit;
            if (isIntersectTriangle(ray, triangles[i], tempHit)) {
                if (tempHit.t < closestT) {
                    closestT = tempHit.t;
                    rayHit.t = tempHit.t;
                    rayHit.sphereIndex = -1;
                    rayHit.triangleIndex = i;
                }
            }
        }
        
        for (int i = 0; i < spheres.length(); i++) {
            RayHit tempHit = rayHit;
            if (isIntersectSphere(ray, spheres[i], tempHit)) {
                if (tempHit.t < closestT) {
                    closestT = tempHit.t;
                    rayHit.t = tempHit.t;
                    rayHit.sphereIndex = i;
                    rayHit.triangleIndex = -1;
                }
            }
        }

        // Debug: visualize t and sphere index
        // imageStore(img_output, pixel, vec4(rayHit.t/20, rayHit.t/20, rayHit.t/20, 1.0));
        // imageStore(img_output, pixel, vec4(float(rayHit.sphereIndex)/20.0, float(rayHit.sphereIndex)/20.0, float(rayHit.sphereIndex)/20.0, 1.0));
        // return;

        if (rayHit.sphereIndex < 0 && rayHit.triangleIndex < 0) {
            accumulatedColor += vec3(backgroundColourAndNumBounces.xyz) * rayHit.colourAccumulation * accumulatedWeight;
            break;
        }

        float reflectivity = 0.0;
        vec3 hitPoint = ray.origin + ray.direction * rayHit.t;
        vec3 materialColor = vec3(1.0);
        vec3 emmisiveColor = vec3(0.0);
        vec3 normal = vec3(0.0);
        
        if (rayHit.triangleIndex < 0 && rayHit.sphereIndex >=0) {
            Sphere hitSphere = spheres[rayHit.sphereIndex];
            normal = normalize(hitPoint - hitSphere.centre.xyz);
            reflectivity = hitSphere.material.materialColour.w;
            materialColor = hitSphere.material.materialColour.xyz;
            emmisiveColor = hitSphere.material.emmissiveColor.xyz * hitSphere.material.emmissiveColor.w;
        }
        
        if (rayHit.sphereIndex < 0 && rayHit.triangleIndex >=0) {
           Triangle hitTriangle = triangles[rayHit.triangleIndex];
           normal = hitTriangle.normal;
           reflectivity = hitTriangle.material.materialColour.w;
           materialColor = hitTriangle.material.materialColour.xyz;
           emmisiveColor = hitTriangle.material.emmissiveColor.xyz * hitTriangle.material.emmissiveColor.w;
        }
        
        // Accumulate emission + material color
        rayHit.lightAccumulation += emmisiveColor * accumulatedWeight;
        accumulatedColor += (rayHit.colourAccumulation) * rayHit.lightAccumulation;
        rayHit.colourAccumulation *= materialColor;
        
        // Compute new ray direction (diffuse + specular)
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
