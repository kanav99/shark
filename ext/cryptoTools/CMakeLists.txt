cmake_minimum_required(VERSION 3.16)
project(cryptoTools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -maes -Wno-ignored-attributes -march=native")

add_library(${PROJECT_NAME} STATIC
    cryptoTools/Common/Defines.cpp
    cryptoTools/Common/Log.cpp
    cryptoTools/Crypto/AES.cpp
    cryptoTools/Crypto/PRNG.cpp
    cryptoTools/Crypto/blake2/sse/blake2b.cpp
    cryptoTools/Crypto/blake2/sse/blake2bp.cpp
    cryptoTools/Crypto/blake2/sse/blake2xb.cpp
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)