# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("mvdrone")

# Déclare les dépendences de buid
set(CMAKE_CXX_STANDARD 17)

set( TCLAP_INCLUDE_DIR "" CACHE PATH "The path containing tclap/TclpCmd.h")
set( LIBAV_INCLUDE_DIR "" CACHE PATH "The path containing libavcodec/avcodec.h")
set( LIBAV_LIBRARY_DIR "" CACHE PATH "The path containing avcodec.a or avcodec.lib")

enable_testing()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message("debug mode")
endif()




#add_definitions(-DDEBUG_VIDEO_CLIENT)
add_definitions(-DDEBUG_VIDEO_STAGING)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package( OpenCV REQUIRED)
find_package( Qt5Network REQUIRED)
find_package( Qt5Gamepad REQUIRED)
find_package( Qt5Gui REQUIRED)

include_directories( 
	${OpenCV_INCLUDE_DIRS}
	${TCLAP_INCLUDE_DIR}
	${LIBAV_INCLUDE_DIR}
)

link_directories(
	${LIBAV_LIBRARY_DIR}
)

set ( LIBAV_LIBS avcodec avutil avformat swscale)

# Include sub-projects.
add_subdirectory("libardrone")
add_subdirectory("libcontrol")
add_subdirectory("ardrone_utils")
add_subdirectory("ardrone_test")
add_subdirectory("ardrone_keyboard")

