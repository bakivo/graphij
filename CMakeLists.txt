cmake_minimum_required(VERSION 3.19)
project(graphij C)

set(CMAKE_C_STANDARD 11)

add_executable(graphij main.c include/trees.h trees.c)