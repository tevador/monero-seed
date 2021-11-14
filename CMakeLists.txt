# Copyright (c) 2020, tevador <tevador@gmail.com>

cmake_minimum_required(VERSION 3.5)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "Setting default build type: ${CMAKE_BUILD_TYPE}")
endif()

project(monero-seed)

add_executable(${PROJECT_NAME}
src/argon2/blake2/blake2b.c
src/argon2/argon2.c
src/argon2/core.c
src/argon2/ref.c
src/galois_field.cpp
src/gf_elem.cpp
src/gf_poly.cpp
src/main.cpp
src/monero_seed.cpp
src/pbkdf2.c
src/reed_solomon_code.cpp
src/secure_random.cpp
src/wordlist.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
