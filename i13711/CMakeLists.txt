# cmake template

# CMake compatibility issues.
cmake_minimum_required(VERSION 2.6)

# allow more human readable "if then else" constructs
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# main
project(sf)

# project version
set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 1)
set(${PROJECT_NAME}_PATCH_LEVEL 0)

enable_language(RC)
set(CMAKE_RC_COMPILER rc)
set(CMAKE_RC_COMPILER_ARG1 -cif8) 

# project options
option(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON)
option(INSTALL_DOC "Set to OFF to skip build/install Documentation" ON)

if( NOT CMAKE_BUILD_TYPE)
	set( CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

add_subdirectory(src)
add_subdirectory(demo)
add_subdirectory(test)
