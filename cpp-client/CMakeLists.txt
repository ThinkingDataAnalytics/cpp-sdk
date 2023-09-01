cmake_minimum_required(VERSION 3.22)
project(untitled1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include_directories(cpp/include)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8) # x64平台
        include_directories(cpp/thirdparty/x64/curl/include cpp/thirdparty/x64/zlib/include cpp/thirdparty/x64/sqlite/include)
        link_directories(cpp/thirdparty/x64/curl/lib cpp/thirdparty/x64/zlib/lib cpp/thirdparty/x64/sqlite/lib)
    else() # Win32平台
        include_directories(cpp/thirdparty/x86/curl/include cpp/thirdparty/x86/zlib/include cpp/thirdparty/x86/sqlite/include)
        link_directories(cpp/thirdparty/x86/curl/lib cpp/thirdparty/x86/zlib/lib cpp/thirdparty/x86/sqlite/lib)
    endif()
    add_library(thinkingdata SHARED cpp/src/ta_analytics_sdk.cpp cpp/src/ta_cpp_helper.cpp cpp/src/ta_cpp_network.cpp cpp/src/ta_cpp_utils.cpp cpp/src/ta_sqlite.cpp cpp/src/ta_timer.cpp cpp/src/ta_event_task.cpp cpp/src/ta_cpp_send.cpp cpp/src/ta_json_object.cpp cpp/src/ta_cJSON.c)
    target_link_libraries(thinkingdata libcurl sqlite3 zlibwapi)
endif()

if (CMAKE_HOST_APPLE)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    add_library(thinkingdata SHARED cpp/src/ta_analytics_sdk.cpp cpp/src/ta_cpp_helper.cpp cpp/src/ta_cpp_network.cpp cpp/src/ta_cpp_utils.cpp cpp/src/ta_sqlite.cpp cpp/src/ta_timer.cpp cpp/src/ta_event_task.cpp cpp/src/ta_cpp_send.cpp cpp/src/ta_json_object.cpp cpp/src/ta_cJSON.c cpp/src/ta_mac_tool.mm)
    target_link_libraries(thinkingdata curl z sqlite3 ${COCOA} ${IOKIT})
endif()



add_executable(untitled1 main.cpp)
target_link_libraries(untitled1 thinkingdata)
