cmake_minimum_required(VERSION 3.16)
project(todo-qt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
  add_compile_options(/utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(todo-qt
  src/main.cpp
  src/mainwindow.cpp
  src/mainwindow.h
  src/todostore.cpp
  src/todostore.h
)

target_link_libraries(todo-qt PRIVATE Qt6::Widgets)

if(WIN32)
  set_target_properties(todo-qt PROPERTIES WIN32_EXECUTABLE ON OUTPUT_NAME "todo-qt")
endif()
