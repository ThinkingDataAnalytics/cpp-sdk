cmake_minimum_required(VERSION 3.22)
project(untitled1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include_directories(cpp/include)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8) # x64平台
        include_directories(cpp/thirdparty/x64/curl/include cpp/thirdparty/x64/zlib/include cpp/thirdparty/x64/sqlite/include cpp/thirdparty/x64/openssl/include)
        link_directories(cpp/thirdparty/x64/curl/lib cpp/thirdparty/x64/zlib/lib cpp/thirdparty/x64/sqlite/lib cpp/thirdparty/x64/openssl/lib)
#        link_directories(cpp/thirdparty/x64/curl/staticlib cpp/thirdparty/x64/zlib/staticlib cpp/thirdparty/x64/sqlite/staticlib cpp/thirdparty/x64/openssl/staticlib)
    else() # Win32平台
        include_directories(cpp/thirdparty/x86/curl/include cpp/thirdparty/x86/zlib/include cpp/thirdparty/x86/sqlite/include cpp/thirdparty/x86/openssl/include)
        link_directories(cpp/thirdparty/x86/curl/lib cpp/thirdparty/x86/zlib/lib cpp/thirdparty/x86/sqlite/lib cpp/thirdparty/x86/openssl/lib)
#        link_directories(cpp/thirdparty/x86/curl/staticlib cpp/thirdparty/x86/zlib/staticlib cpp/thirdparty/x86/sqlite/staticlib cpp/thirdparty/x86/openssl/staticlib)
    endif()
    add_library(thinkingdata SHARED cpp/src/ta_analytics_sdk.cpp cpp/src/ta_cpp_helper.cpp cpp/src/ta_cpp_network.cpp cpp/src/ta_cpp_utils.cpp cpp/src/ta_sqlite.cpp cpp/src/ta_timer.cpp cpp/src/ta_event_task.cpp cpp/src/ta_cpp_send.cpp cpp/src/ta_json_object.cpp cpp/src/ta_cJSON.c cpp/src/ta_encrypt.cpp cpp/src/ta_calibrated_time.cpp cpp/src/ta_flush_task.cpp)
    target_link_libraries(thinkingdata libcurl sqlite3 zlibwapi libssl libcrypto)
#    add_library(thinkingdata STATIC cpp/src/ta_analytics_sdk.cpp cpp/src/ta_cpp_helper.cpp cpp/src/ta_cpp_network.cpp cpp/src/ta_cpp_utils.cpp cpp/src/ta_sqlite.cpp cpp/src/ta_timer.cpp cpp/src/ta_event_task.cpp cpp/src/ta_cpp_send.cpp cpp/src/ta_json_object.cpp cpp/src/ta_cJSON.c cpp/src/ta_encrypt.cpp cpp/src/ta_calibrated_time.cpp cpp/src/ta_flush_task.cpp)
#    target_link_libraries(thinkingdata wldap32 ws2_32 Crypt32 Normaliz libssl libcrypto libcurl_a sqlite3 zlibstat)
endif()

if (CMAKE_HOST_APPLE)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_LIBRARIES})
    add_library(thinkingdata SHARED cpp/src/ta_analytics_sdk.cpp cpp/src/ta_cpp_helper.cpp cpp/src/ta_cpp_network.cpp cpp/src/ta_cpp_utils.cpp cpp/src/ta_sqlite.cpp cpp/src/ta_timer.cpp cpp/src/ta_event_task.cpp cpp/src/ta_cpp_send.cpp cpp/src/ta_json_object.cpp cpp/src/ta_cJSON.c cpp/src/ta_mac_tool.mm cpp/src/ta_encrypt.cpp cpp/src/ta_calibrated_time.cpp cpp/src/ta_flush_task.cpp)
    target_link_libraries(thinkingdata curl z sqlite3 ${OPENSSL_LIBRARIES} ${COCOA} ${IOKIT})
endif()



add_executable(untitled1 main.cpp)
target_link_libraries(untitled1 thinkingdata)
