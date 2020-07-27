cmake_minimum_required(VERSION 3.10)
project(mid_svr)

set(CMAKE_CXX_STANDARD 14)

add_executable(mid_svr main.cpp WebServer.cpp http_conn.cpp Utils.cpp)

TARGET_LINK_LIBRARIES(mid_svr json)
TARGET_LINK_LIBRARIES(mid_svr curl)
TARGET_LINK_LIBRARIES(mid_svr pcap)
TARGET_LINK_LIBRARIES(mid_svr pthread)
TARGET_LINK_LIBRARIES(mid_svr boost_timer)
