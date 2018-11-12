# CMakeList.txt : CMake project for mvlar, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(ARDRONE_TEST_VERSION_MAJOR 0)
set(ARDRONE_TEST_VERSION_MINOR 1)

include_directories (
    "${PROJECT_SOURCE_DIR}/libardrone/include"
	"${CMAKE_CURRENT_BINARY_DIR}"
)

configure_file (
	"build_info.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/build_info.h"
)


# Add source to this project's executable.
add_executable (ardrone_test main.cpp video_test.h cuda_test.h staging_test.h atcmd_test.h wifi_test.h)

if(NOT WIN32)
	set( LINK_FS stdc++fs pthread)
endif ()

# Ajoute le lien a ma librairie
target_link_libraries (ardrone_test libardrone Qt5::Network ${OpenCV_LIBS} ${LINK_FS} ${LIBAV_LIBS} ) 

