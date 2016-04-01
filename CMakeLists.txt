cmake_minimum_required(VERSION 2.8)
project(htmlex)

if(MSVC)
  set_target_properties(${testname}
    PROPERTIES LINK_FLAGS -ENTRY:"mainCRTStartup")
endif()

add_executable(htmlex
  htmlex.c
  macros.c
  stream.c
  tags.c)
