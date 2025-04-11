cmake_minimum_required(VERSION 3.10)
project(RayTracingInOneWeekend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(raytracer RayTracerApp/RayTracerApp.cpp)