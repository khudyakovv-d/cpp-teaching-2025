cmake_minimum_required(VERSION 3.10.0)
project(lab0a VERSION 0.1.0 LANGUAGES C CXX)

add_executable(lab0a main.cpp src/module2/module2.h src/module2/module2.cpp)

target_include_directories(lab0a PUBLIC src/module1 src/module2)

add_subdirectory(src/module1)

target_link_libraries(lab0a PUBLIC module1)