# CMakeList.txt : CMake project for mvlar, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Essaie d'include OpenCV
find_package( OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")


include_directories (
    "${PROJECT_SOURCE_DIR}/libardrone/include"
    "${FFMPEG_INCLUDE_PATH}"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Network)

link_directories(${FFMPEG_LIBRARY_DIR})

# Add source to this project's executable.
add_executable (ardrone_test "mvlar.cpp" "mvlar.h")

# Ajoute le lien a ma librairie
target_link_libraries (ardrone_test libardrone Qt5::Network ${OpenCV_LIBS}) 

