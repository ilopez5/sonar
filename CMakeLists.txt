# specifies minimum version
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# specify project name
project(sonar)

# set install directory
set(INSTALL_DIR $ENV{INSTALL_DIR})
message(INFO "Installation Directory ${INSTALL_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -L${INSTALL_DIR}/lib -L${INSTALL_DIR}/lib64 -I${INSTALL_DIR}/include -I${INSTALL_DIR}/src/labios1.0/include -L${INSTALL_DIR}/src/labios1.0/build")

# specify executable name
add_executable(sonar sonar.cpp sonar.h)

# link appropriate libraries
target_link_libraries(sonar -llabios -lmpi -lmemcached -lnats)