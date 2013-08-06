# cmake template

# cmake compatibility issues.

cmake_minimum_required(VERSION 2.6)
mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

# allow more human readable "if then else" constructs

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# main 

project(project_name)

# project version
set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 1)
set(${PROJECT_NAME}_PATCH_LEVEL 0)

# project options
#option(BUILD_SHARED_LIBS "set to OFF to build static libraries" ON)
#option(INSTALL_DOC "set to OFF to skip build/install documenation" ON)

if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# set default install prefix to project root directory
if( CMAKE_INSTALL_PREFIX STREQUAL "/usr/local" )
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}")
endif()

# include directories
include_directories("${PROJECT_SOURCE_DIR}/include")

# install include files
#install(DIRECTORY "${}"

# require proper c++/c++11
add_definitions("-Wall -std=c++11 -pedantic")

# get list of all source files
#aux_source_directory(src)

add_subdirectory(src)

