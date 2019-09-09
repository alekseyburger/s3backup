#
#  cmake <CMakeLists.txt dir> -DBOOST_ROOT=<path to boost>
#

cmake_minimum_required(VERSION 3.2)

set(name mysrv)
project(${name} CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_INCLUDE_DIR /opt/boost_1_69_0)
set(Boost_LIBRARY_DIR /opt/boost_1_69_0/stage/lib)

# we will use static libs
set(Boost_USE_STATIC_LIBS ON)

# Boost::log required Boost version >= 1.54.0 
find_package(Boost 1.54.0 REQUIRED COMPONENTS log regex)
find_package(Threads)
# Locate the AWS SDK for C++ package.
# Requires that you build with:
#   -DCMAKE_PREFIX_PATH=/path/to/sdk_install
find_package(AWSSDK REQUIRED COMPONENTS s3)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/inc)
# The executable name and its sourcefiles
set(SRC_FILES
    src/main.cpp
    src/s3_application.cpp)

add_executable(${name} ${SRC_FILES})
target_link_libraries(${name} ${Boost_LIBRARIES} Threads::Threads)

# The libraries used by your executable.
# "aws-cpp-sdk-s3" is just an example.
target_link_libraries(${name} ${AWSSDK_LINK_LIBRARIES})

configure_file(src/${name}.service ${CMAKE_BINARY_DIR}/${name}.service)

# deploy service
add_custom_target(deploy
    sudo cp ${CMAKE_BINARY_DIR}/${name}.service /etc/systemd/system
)

# remove service
add_custom_target(undeploy
    sudo rm /etc/systemd/system/${name}.service
)
