# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("ardrone" CXX)
# Déclare les dépendences de buid
set(CMAKE_CXX_STANDARD 17)


set( TCLAP_INCLUDE_DIR "" CACHE PATH "The path containing tclap/TclpCmd.h")
set( LIBAV_INCLUDE_DIR "" CACHE PATH "The path containing libavcodec/avcodec.h")
set( LIBAV_LIBRARY_DIR "" CACHE PATH "The path containing avcodec.a or avcodec.lib")

set(GLEW_INCLUDE_PATH "" CACHE PATH "The directory that contains GL/glew.h" )
set(GLEW_LIBRARY "" CACHE FILEPATH "libglew.a or glew.lib" )

set(GLM_INCLUDE_PATH "" CACHE PATH "The directory that contains glm/glm.hpp" )
set(SOIL_INCLUDE_PATH "" CACHE PATH "The directory that contains SOIL.h" )
set(SOIL_LIBRARY "" CACHE PATH "The directory that contains SOIL.lib" )

set(ASSIMP_INCLUDE_PATH "" CACHE PATH "The directory that contains assimp/assimp.h" )
set(ASSIMP_LIBRARY "" CACHE PATH "The directory that contains assimp.lib" )
set(STB_INCLUDE_PATH "" CACHE PATH "The directory that contains stb_image.h")

set(GLFW_INCLUDE_PATH "" CACHE PATH "The directory that contains GLFW/glfw3.h" )
set(GLFW_LIBRARY "" CACHE PATH "The directory that contains glfw3.lib" )
set( ASIO_INCLUDE_DIR "" CACHE PATH "The path containing asio/asio.hpp")
add_definitions(-DASIO_STANDALONE)
if(WIN32)
	add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING -D_WIN32_WINNT=0xA00 -DUNICODE)
endif()
enable_testing()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message("debug mode")
endif()



include_directories(
	${GLEW_INCLUDE_PATH}
	${GLFW_INCLUDE_PATH}
	${GLM_INCLUDE_PATH}
        libutils
)
#add_definitions(-DWITH_STB_IMAGE)

link_directories(
	${GLEW_LIBRARY}
	${GLFW_LIBRARY}
)


#add_definitions(-DDEBUG_VIDEO_CLIENT)
add_definitions(-DDEBUG_VIDEO_STAGING)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package( OpenGL REQUIRED)
find_package( OpenCV REQUIRED)
find_package( Qt5Network REQUIRED)
find_package( Qt5Gamepad REQUIRED)

include_directories( 
	${OpenCV_INCLUDE_DIRS}
	${TCLAP_INCLUDE_DIR}
	${LIBAV_INCLUDE_DIR}
	${ASIO_INCLUDE_DIR}
	${QT_INCLUDES}
	./libutils
)

link_directories(
	${LIBAV_LIBRARY_DIR}
)
if(WIN32)
	set ( GL_LIBS opengl32 glew32d glfw3)
else()
	set ( GL_LIBS GL GLEW glfw atomic )
	set ( CMAKE_CXX_FLAGS -march=native)
endif()

set ( LIBAV_LIBS avcodec avutil avformat swscale)

# Include sub-projects.
add_subdirectory("imgui")
add_subdirectory("libardrone")
add_subdirectory("libcontrol")
add_subdirectory("ardrone_gui")

