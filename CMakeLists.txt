# 1. Minimum version ensures compatibility with modern features
cmake_minimum_required(VERSION 3.10)
# 2. Defines the project name
project(app)
# 3. Sets the C++ standard to 20 for modern features
set(CMAKE_CXX_STANDARD 20)
# 4. Creates the executable target
add_executable(app main.cpp)
