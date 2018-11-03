# CMakeList.txt : Pour le librairie libardrone
cmake_minimum_required (VERSION 3.8)

project (libardrone VERSION 0.0.1 LANGUAGES CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Crée ma target de la librairie
add_library (libardrone src/drone_client.cpp src/navdata_client.cpp src/thread.cpp src/video_client.cpp)



