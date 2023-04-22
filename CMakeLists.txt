cmake_minimum_required(VERSION 3.4)

set(CMAKE_BUILD_TYPE "Debug")

project(rpmsg_echo_test CXX)
add_executable(rpmsg_echo_test echo_test.cpp)
target_compile_options(rpmsg_echo_test PUBLIC -O2 -Wall -lstdc++fs)
target_compile_features(rpmsg_echo_test PUBLIC cxx_std_17)
