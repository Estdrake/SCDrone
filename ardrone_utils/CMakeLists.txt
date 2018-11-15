# CMakeList.txt : CMake project for mvlar, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(ARDRONE_UTILS_VERSION_MAJOR 0)
set(ARDRONE_UTILS_VERSION_MINOR 1)

include_directories (
    "${PROJECT_SOURCE_DIR}/libardrone/include"
)


# Add source to this project's executable.
add_executable (ardrone_utils main.cpp control.h demo_1.h)

if(NOT WIN32)
	set( LINK_FS stdc++fs pthread)
endif ()

# Ajoute le lien a ma librairie
target_link_libraries (ardrone_utils libardrone Qt5::Network Qt5::Gamepad Qt5::Gui ${OpenCV_LIBS} ${LINK_FS} ${LIBAV_LIBS} ) 

install (TARGETS ardrone_utils DESTINATION bin)

