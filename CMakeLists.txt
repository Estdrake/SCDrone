# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("mvdrone")

# Déclare les dépendences de buid
set(CMAKE_CXX_STANDARD 11)

enable_testing()


find_package(Boost REQUIRED COMPONENTS system thread system atomic log)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
	link_directories(${Boost_LIBRARY_DIRS})
	set(Boost_USE_MULTITHREAD ON)
	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_DEBUG ON)
endif()


# Include sub-projects.

add_subdirectory("libardrone")
add_subdirectory("libcontrol")
add_subdirectory ("ardrone_test")

