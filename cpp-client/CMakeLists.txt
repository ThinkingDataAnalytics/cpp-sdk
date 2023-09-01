cmake_minimum_required(VERSION 3.23)
project(untitled1)

set(CMAKE_CXX_STANDARD 11)

include_directories(cpp/include)

if(WIN32)
    include_directories(cpp/thirdparty/curl/include cpp/thirdparty/zlib/include cpp/thirdparty/sqlite/include)
    link_directories(cpp/thirdparty/curl/lib cpp/thirdparty/zlib/lib cpp/thirdparty/sqlite/lib cpp/thirdparty/tasqlite/lib)
    add_library(thinkingdata cpp/src/ta_analytics_sdk.cpp cpp/src/ta_cpp_helper.cpp cpp/src/ta_cpp_network.cpp cpp/src/ta_cpp_utils.cpp cpp/src/ta_sqlite.cpp cpp/src/ta_timer.cpp cpp/src/ta_event_task.cpp cpp/src/ta_cpp_send.cpp cpp/src/ta_json_object.cpp cpp/src/ta_cJSON.c)
    target_link_libraries(thinkingdata libcurl zlib libsqlite_share)
endif()

if (CMAKE_HOST_APPLE)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    add_library(thinkingdata cpp/src/ta_analytics_sdk.cpp cpp/src/ta_cpp_helper.cpp cpp/src/ta_cpp_network.cpp cpp/src/ta_cpp_utils.cpp cpp/src/ta_sqlite.cpp cpp/src/ta_timer.cpp cpp/src/ta_event_task.cpp cpp/src/ta_cpp_send.cpp cpp/src/ta_json_object.cpp cpp/src/ta_cJSON.c cpp/src/ta_mac_tool.mm)
    target_link_libraries(thinkingdata curl z sqlite3 ${COCOA} ${IOKIT})
endif()



add_executable(untitled1 main.cpp)
target_link_libraries(untitled1 thinkingdata)
