cmake_minimum_required (VERSION 3.8)

# Crée ma librairie 
add_library (libcontrol "tracking.cpp" "tracking.h" "filter.cpp" "filter.h" "common_headers.h")
target_link_libraries(libcontrol ${OpenCV_LIBS} )
