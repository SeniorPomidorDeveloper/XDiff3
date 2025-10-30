cmake_minimum_required(VERSION 3.14)
project(xdiff3_cli CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

<<<<<<< HEAD
# основная динамическая либа
add_library(xdiff3 SHARED src/xDiff3.cpp)
target_include_directories(xdiff3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(xdiff3 PROPERTIES OUTPUT_NAME xdiff3)
=======
add_library(xdiff3 SHARED
    src/xDiff3.cpp
)
>>>>>>> 4b06621800aef0ce4747503dcd605fe73896efc6

# bench-обёртка для графиков (ctypes)
add_library(xdiff3_bench SHARED bench/bench.cpp)
target_include_directories(xdiff3_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(xdiff3_bench PRIVATE xdiff3)
set_target_properties(xdiff3_bench PROPERTIES OUTPUT_NAME xdiff3_bench)

# CLI — использует только чистую либу и твой main.cpp без изменений
add_executable(xdiff3_cli main.cpp)
target_include_directories(xdiff3_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(xdiff3_cli PRIVATE xdiff3)

# rpath, чтобы бинарь и bench.so видели libxdiff3.so рядом
if(APPLE)
  set_target_properties(xdiff3_cli xdiff3_bench PROPERTIES BUILD_RPATH "@loader_path")
elseif(UNIX)
  set_target_properties(xdiff3_cli xdiff3_bench PROPERTIES BUILD_RPATH "$ORIGIN")
endif()

# флаги (опционально)
if (MSVC)
  target_compile_options(xdiff3 PRIVATE /O2 /W4 /permissive-)
  target_compile_options(xdiff3_bench PRIVATE /O2 /W4 /permissive-)
  target_compile_options(xdiff3_cli PRIVATE /O2 /W4 /permissive-)
else()
  target_compile_options(xdiff3 PRIVATE -O3 -DNDEBUG -Wall -Wextra -Wpedantic)
  target_compile_options(xdiff3_bench PRIVATE -O3 -DNDEBUG -Wall -Wextra -Wpedantic)
  target_compile_options(xdiff3_cli PRIVATE -O3 -DNDEBUG -Wall -Wextra -Wpedantic)
endif()
