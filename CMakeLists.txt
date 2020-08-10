# specifies minimum version
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# specify project name
project(sonar)

# MPICH
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
set(CMAKE_C_COMPILER mpicc)
set(CMAKE_CXX_COMPILER mpicxx)

# move to install/ directory
set(CMAKE_INSTALL_DIR ~/documents/research/install)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_DIR}/bin)

# specify executable name
add_executable(sonar sonar.cpp sonar.h)
