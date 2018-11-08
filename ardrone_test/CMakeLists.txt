# CMakeList.txt : CMake project for mvlar, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)




include_directories (
    "${PROJECT_SOURCE_DIR}/libardrone/include"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Network)

# Add source to this project's executable.
add_executable (ardrone_test "mvlar.cpp" "mvlar.h")

# Ajoute le lien a ma librairie
target_link_libraries (ardrone_test libardrone Qt5::Network ${OpenCV_LIBS}) 

