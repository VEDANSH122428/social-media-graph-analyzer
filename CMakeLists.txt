cmake_minimum_required(VERSION 3.10)
project(SocialNetworkGraphAnalyzer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB SOURCES cp/*.cpp)
add_executable(analyzer ${SOURCES})
target_include_directories(analyzer PRIVATE cp terminal)

if(WIN32)
  target_link_libraries(analyzer ws2_32)
endif()