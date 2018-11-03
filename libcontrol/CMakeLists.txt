cmake_minimum_required (VERSION 3.8)

# Essaie d'include OpenCV
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Crée ma librairie 
add_library (libcontrol "tracking.cpp" "tracking.h" "common_headers.h")
target_link_libraries(libcontrol ${OpenCV_LIBS} )
