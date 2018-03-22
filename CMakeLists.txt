cmake_minimum_required(VERSION 3.9)
project(Zeroconf_course_project)

set(CMAKE_CXX_STANDARD 98)

include_directories(.)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")

add_executable(Zeroconf_course_project
        main.c zeroconf.h zeroconf.c)
