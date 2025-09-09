# GLFW Ray Tracer

A raytracer using GLFW for window creation.

This project is licensed under the MIT License, see [LICENSE](/LICENSE).

---

## Features

- Creation of a window via GLFW, that will display a background colour.

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
git clone https://github.com/AngusWebbHammond/GLFW-Ray-Tracer.git
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

4. **Build the project in Debug**:

```bash
cmake --build .
```

5. **Run the executable**:

```bash
cd Debug
.\main.exe
```
