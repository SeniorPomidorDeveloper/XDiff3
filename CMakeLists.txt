cmake_minimum_required(VERSION 3.14)
project(xdiff3_cli CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xdiff3 STATIC
    src/xDiff3.cpp
)

target_include_directories(xdiff3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(xdiff3_cli
    main.cpp
)

target_link_libraries(xdiff3_cli PRIVATE xdiff3)

if (MSVC)
  target_compile_options(xdiff3 PRIVATE /O2 /W4 /permissive-)
  target_compile_options(xdiff3_cli PRIVATE /O2 /W4 /permissive-)
else()
  target_compile_options(xdiff3 PRIVATE -O3 -DNDEBUG -march=native -Wall -Wextra -Wpedantic)
  target_compile_options(xdiff3_cli PRIVATE -O3 -DNDEBUG -march=native -Wall -Wextra -Wpedantic)
endif()
