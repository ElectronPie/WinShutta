cmake_minimum_required(VERSION 3.27)

project(glad)

add_library(${PROJECT_NAME}
    include/glad/glad.h
    include/KHR/khrplatform.h
    src/glad.c
)
target_include_directories(glad PUBLIC include)
