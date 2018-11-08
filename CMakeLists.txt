# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("mvdrone")

# Déclare les dépendences de buid
set(CMAKE_CXX_STANDARD 17)

enable_testing()

# Essaie d'include OpenCV
find_package( OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )


# Include sub-projects.

add_subdirectory("libardrone")
add_subdirectory("libcontrol")
add_subdirectory ("ardrone_test")

