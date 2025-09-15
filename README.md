# GLFW Ray Tracer

A 3D ray tracer, built using GLFW and ImGui. Built to run either on the CPU or the GPU through Compute Shaders.

|   Rendering Type   |                             Specular Reflections                             |                           Diffuse Reflections                            |
| :----------------: | :--------------------------------------------------------------------------: | :----------------------------------------------------------------------: |
|        CPU         |    ![Lighter Background](./assets/docs/Application%20View%20Specular.jpg)    |    ![Diffuse Bounces](./assets/docs/Application%20View%20Diffuse.jpg)    |
| GPU Compute Shader | ![Lighter Background](./assets/docs/Application%20View%20Specular%20GPU.jpg) | ![Diffuse Bounces](./assets/docs/Application%20View%20Diffuse%20GPU.jpg) |

This project is licensed under the MIT License, see [LICENSE](/LICENSE).

---

## Features

- Diffuse Reflections, using Lambert's cosine law to favour random values near the normal.
- Specular Reflections.
- Realtime updating of spheres.
- Accumulation of frames.
- Multithreading of the CPU to parallelize the ray casting from the camera.
- Utilisation of the GPU through a Compute Shader.

## Dependencies

- [GLFW](https://www.glfw.org/) – Window and input management
- [Glad](https://github.com/Dav1dde/glad) – OpenGL loader
- [GLM](https://github.com/g-truc/glm) – Mathematics library
- [ImGui](https://github.com/ocornut/imgui) – GUI library

> All dependencies are included either in the CMake project or the deps folder. No manual linking is required.

---

## Building the Project

The project uses **CMake** for an easy build process:

1. **Clone the repository**:

   ```bash
   git clone --recurse-submodules https://github.com/AngusWebbHammond/GLFW-Ray-Tracer.git
   cd GLFW-Ray-Tracer
   ```

2. **Create a build directory**:

   ```bash
   mkdir build
   cd build
   ```

3. **Run CMake** to configure the project:

   ```bash
   cmake ..
   ```

4. **Build the project in Release**:

   ```bash
   cmake --build . --config Release
   ```

   > It is advised to run in release, due to the number of rays being shot out each frame.

5. **Run the executable**:

   ```bash
   cd Release
   ./main.exe
   ```
