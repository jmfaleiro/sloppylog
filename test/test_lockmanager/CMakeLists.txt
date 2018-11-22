file(GLOB SRCS *.cpp)

add_executable(test_lockmanager ${SRCS})

include_directories(${CMAKE_HOME_DIRECTORY}/include)

target_link_libraries(test_lockmanager
  libsloppydb
  libgtest
  libgmock
)
