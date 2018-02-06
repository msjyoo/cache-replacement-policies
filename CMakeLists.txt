cmake_minimum_required(VERSION 3.9)
project(cache_replacement_policies C)

set(CMAKE_C_STANDARD 11)

add_executable(cache_replacement_policies main.c model/disk_model.c model/cache_model.c model/cache_model.h policies/lru.c policies/lru.h)

add_executable(tests tests/tests.c)
target_link_libraries(tests cmocka)