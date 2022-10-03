cmake_minimum_required(VERSION 3.10)

project(CacheProbe VERSION 1.0 DESCRIPTION "CacheProbe")

add_compile_options(-Wall -Wextra -pedantic -O3)
include_directories("include/")
add_executable(cache_probe cache_probe.cpp)
