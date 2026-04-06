cmake_minimum_required(VERSION 3.10.0)
project (EmbeddedC)
set(CMAKE_BUILD_TYPE Release)
include_directories(inc)
set(SOURCES src/main.c)
file(GLOB SOURCES "src/*.c")
add_executable(managers ${SOURCES})
add_custom_command(TARGET managers POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:managers> ${CMAKE_SOURCE_DIR}/build/managers.exe
)