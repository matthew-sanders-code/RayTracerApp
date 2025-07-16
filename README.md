# RayTracerApp
A simple raytracer built with C++. This ray tracer was built by following Peter Shirley's [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html) series. All core concepts, structure, and many code snippets originate from the guide.


## Features
- Renders spheres
- Support for:
  - Shadows
  - Reflections
- Image output in BMP
- Multithreading

## Built With

- C++17
- CMake
- SFML

## Installation
1. git clone https://github.com/matthew-sanders-code/RayTracerApp.git
2. cd RayTracerApp
3. cmake --build build --config release
4. cd build/Release
5. ./RayTracerApp 

## Future Features
- Implement acceleration structures (BVH)
