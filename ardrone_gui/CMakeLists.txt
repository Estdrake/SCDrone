# CMakeList.txt : CMake project for mvlar, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(ARDRONE_GUI_VERSION_MAJOR 0)
set(ARDRONE_GUI_VERSION_MINOR 1)

include_directories (
    "${PROJECT_SOURCE_DIR}/libardrone/include"
    "${PROJECT_SOURCE_DIR}/libcontrol/"
)


include_directories (${PROJECT_SOURCE_DIR}/imgui)

# Add source to this project's executable.
add_executable (ardrone_gui main.cpp rgb_player.h shaders.cpp shaders.h)

if(NOT WIN32)
	set( LINK_FS stdc++fs pthread)
endif ()

# Ajoute le lien a ma librairie
target_link_libraries (ardrone_gui imgui libcontrol libardrone Qt5::Network ${OpenCV_LIBS} ${LINK_FS} ${LIBAV_LIBS} ${GL_LIBS} ) 

file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
