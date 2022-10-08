cmake_minimum_required(VERSION 3.10)

project(CacheProbe VERSION 1.0 DESCRIPTION "CacheProbe")

include_directories("include/")
add_executable(cache_probe cache_probe.cpp)
target_compile_options(cache_probe PUBLIC -Wall -Wextra -pedantic -O1)
string(REGEX MATCH ".*amd.*" is_arm ${CMAKE_HOST_SYSTEM_PROCESSOR})
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "aarch64")
	message("Configuring for ARM ...")
	target_compile_options(cache_probe PUBLIC -DARM)
endif()
