cmake_minimum_required(VERSION 3.4)

set(CMAKE_BUILD_TYPE "Debug")

project(rpmsg_echo_test CXX)
add_executable(rpmsg_echo_test echo_test.cpp com.cpp)
target_compile_options(rpmsg_echo_test PUBLIC -O2 -Wall)
target_compile_features(rpmsg_echo_test PUBLIC cxx_std_20)
target_link_libraries(rpmsg_echo_test pthread stdc++fs)