cmake_minimum_required(VERSION 3.10)
project(RayTracingInOneWeekend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Tell SFML to use static libraries
set(SFML_STATIC_LIBRARIES TRUE)

# Set the path to the SFML CMake config directory (adjust this if needed)
set(SFML_DIR "C:/SFML-3.0.0/lib/cmake/SFML")

# Find SFML static components
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

# Add your executable
add_executable(raytracer RayTracerApp/RayTracerApp.cpp)

# Define SFML_STATIC so headers behave correctly
target_compile_definitions(raytracer PRIVATE SFML_STATIC)

# Link to SFML libraries
target_link_libraries(raytracer PRIVATE SFML::Graphics SFML::Window SFML::System)

# Link additional dependencies manually (needed for static SFML)
target_link_libraries(raytracer PRIVATE 
    winmm 
    opengl32 
    freetype 
    gdi32 
    ws2_32 
    user32
)