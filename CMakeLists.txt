cmake_minimum_required(VERSION 3.15)
project(pktpy LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-shorten-64-to-32")
endif()

find_package(Python REQUIRED COMPONENTS Development)

add_definitions(-DPK_ENABLE_OS=1)
add_definitions(-DPK_ENABLE_THREADS=0)
add_definitions(-DPK_ENABLE_DLL=0)
add_definitions(-DPK_ENABLE_DETERMINISM=0)
add_definitions(-DPK_ENABLE_WATCHDOG=0)
add_definitions(-DPK_ENABLE_MIMALLOC=0)

file(GLOB_RECURSE POCKETPY_SRC third_party/pocketpy/src/*.c)

add_library(pocketpy STATIC ${POCKETPY_SRC})
target_include_directories(pocketpy PUBLIC third_party/pocketpy/include)

python_add_library(_pocketpy MODULE src/_pocketpy_module.c)
target_link_libraries(_pocketpy PRIVATE pocketpy Python::Python)
target_include_directories(_pocketpy PRIVATE third_party/pocketpy/include)

install(TARGETS _pocketpy LIBRARY DESTINATION pktpy)
