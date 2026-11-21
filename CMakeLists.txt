cmake_minimum_required(VERSION 3.16)
project(StockKeeper VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(EMSCRIPTEN)
  add_compile_options(-fexceptions)
endif()

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(stockkeeper_core
  src/item.cpp
  src/inventory.cpp
  src/storage.cpp
  src/command_line.cpp
  src/text.cpp
  src/money.cpp
)
target_include_directories(stockkeeper_core PUBLIC include)

add_executable(stockkeeper src/main.cpp)
target_link_libraries(stockkeeper PRIVATE stockkeeper_core)

if(EMSCRIPTEN)
  add_executable(stockkeeper_web wasm/bindings.cpp)
  target_link_libraries(stockkeeper_web PRIVATE stockkeeper_core)
  set_target_properties(stockkeeper_web PROPERTIES SUFFIX ".mjs")
  target_link_options(stockkeeper_web PRIVATE
    -lembind -fexceptions -O2
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createStockKeeper
    -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH=1
    "--embed-file=${CMAKE_CURRENT_SOURCE_DIR}/data@/seed")
endif()

option(STOCKKEEPER_BUILD_TESTS "Build unit tests" ON)
if(STOCKKEEPER_BUILD_TESTS)
  enable_testing()
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
  endif()
  add_executable(stockkeeper_tests
    tests/item_test.cpp
    tests/inventory_test.cpp
    tests/storage_test.cpp
    tests/command_line_test.cpp
  )
  target_link_libraries(stockkeeper_tests PRIVATE stockkeeper_core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(stockkeeper_tests)
endif()
