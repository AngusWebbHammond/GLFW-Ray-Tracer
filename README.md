# GLFW Ray Tracer

A 3D ray tracer, built using GLFW and ImGui.

|                      Specular Reflections                       |                     Diffuse Reflections                      |
| :-------------------------------------------------------------: | :----------------------------------------------------------: |
| ![Lighter Background](./assets/docs/Application%20View%202.jpg) | ![Diffuse Bounces](./assets/docs/Application%20View%203.jpg) |

This project is licensed under the MIT License, see [LICENSE](/LICENSE).

---

## Features

- Diffuse Reflections, through uniform random number generation.
- Specular Reflections.
- Realtime updating of spheres.
- Accumulation of frames.

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
