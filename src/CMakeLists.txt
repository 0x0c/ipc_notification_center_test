cmake_minimum_required(VERSION 3.15.2)
project(SocketNotificationCenter)
set(CMAKE_CXX_STANDARD 11)
find_package(Boost 1.70.0 REQUIRED system)

find_package(msgpack REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}" "${Boost_INCLUDE_DIRS}")
add_executable(SocketNotificationCenter SocketNotificationCenter.cpp)
target_link_libraries(SocketNotificationCenter system ${msgpack_LIBRARIES})