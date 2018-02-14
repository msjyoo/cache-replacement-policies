cmake_minimum_required(VERSION 3.9)
project(cache_replacement_policies C)

set(CMAKE_C_STANDARD 11)

set(
        SOURCE_FILES_MODEL
        model/disk_model.c
)

set(
        SOURCE_FILES_POLICIES
        policies/lru.c policies/lru.h
        policies/dummy.c policies/dummy.h
)

#add_executable(
#        cache_replacement_policies
#        ${SOURCE_FILES_MODEL}
#        ${SOURCE_FILES_POLICIES}
#)

set(
        SOURCE_FILES_TESTS
        tests/tests.c
)

add_executable(
        tests
        ${SOURCE_FILES_TESTS}
        ${SOURCE_FILES_MODEL}
        ${SOURCE_FILES_POLICIES}
)

target_link_libraries(tests cmocka)