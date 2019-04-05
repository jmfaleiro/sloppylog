cmake_minimum_required(VERSION 2.8.5)
enable_testing()
set(CMAKE_CXX_STANDARD 17)
project(SloppyDB)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TBB_ROOT "${CMAKE_HOME_DIRECTORY}/lib/tbb")

# add_subdirectory(src)
add_subdirectory(test)

include(${TBB_ROOT}/cmake/TBBBuild.cmake)

# Build Intel TBB with enabled Community Preview Features (CPF).
tbb_build(TBB_ROOT ${TBB_ROOT} CONFIG_DIR TBB_DIR MAKE_ARGS tbb_cpf=1)

find_package(TBB REQUIRED tbb_preview)

file(GLOB MY_SRCS
  "include/*.h"
  "src/*.cpp"
)

list(REMOVE_ITEM MY_SRCS "src/main.cpp")

# Add executable.
add_library(libsloppydb ${MY_SRCS})
add_executable(sloppydb "src/main.cpp")


# Set include dirs and libraries for the executable.
include_directories(include)
target_link_libraries(libsloppydb ${TBB_IMPORTED_TARGETS})
target_link_libraries(sloppydb libsloppydb)

# Try to make lldb work
set(CMAKE_BUILD_TYPE Debug)

# Try to fix warning
cmake_policy(SET CMP0054 OLD)

