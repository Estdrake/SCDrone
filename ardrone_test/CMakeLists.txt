# CMakeList.txt : CMake project for mvlar, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if(MINGW)
    set(MINGW_LIBRARIES ws2_32)
endif()


include_directories ("${PROJECT_SOURCE_DIR}/libardrone/include")

# Add source to this project's executable.
add_executable (ardrone_test "mvlar.cpp" "mvlar.h")

# Ajoute le lien a ma librairie
target_link_libraries (ardrone_test libardrone ${Boost_SYSTEM_LIBRARY} ${Boost_LIBRARIES} ${MINGW_LIBRARIES})

#set (CMAKE_CXX_FLAGS "-Wall -DBOOST_LOG_DYN_LINK")

