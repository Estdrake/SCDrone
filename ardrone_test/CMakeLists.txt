# CMakeList.txt : CMake project for mvlar, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if(MINGW)
    set(MINGW_LIBRARIES ws2_32)
endif()

set( TCLAP_INCLUDE_DIR "" CACHE PATH "The diretory containing tclap/CmdLine.h" ) # Cache path c'est pour les variables qui arrivent du mode cmd

include_directories (
    "${PROJECT_SOURCE_DIR}/libardrone/include"
    "${TCLAP_INCLUDE_DIR}"
)

# Add source to this project's executable.
add_executable (ardrone_test "mvlar.cpp" "mvlar.h")

# Ajoute cette definitions sinon probleme de compilation sur linux
add_definitions( -DBOOST_LOG_DYN_LINK=1 )
# Ajoute le lien a ma librairie
target_link_libraries (ardrone_test libardrone ${Boost_SYSTEM_LIBRARY} ${Boost_LIBRARIES} ${MINGW_LIBRARIES})

#set (CMAKE_CXX_FLAGS "-Wall -DBOOST_LOG_DYN_LINK")

