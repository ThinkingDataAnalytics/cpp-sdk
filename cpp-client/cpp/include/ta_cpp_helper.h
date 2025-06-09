//
//  ta_cpp_helper.hpp
//  hello_mac
//
//  Created by wwango on 2022/3/7.
//

#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

namespace thinkingdata {

    enum class TDLogLevel { TDDEBUG, TDINFO, TDERROR };

#if defined(__APPLE__)
    
    class ta_mac_tool{
    public:
        static void printSDKLog(const string &log);
        static void updateAccount(const char *token, const char *accountId);
        static void updateDistinctId(const char *token, const char *distinctId);
        
        static const char * loadAccount(const char *token);
        static const char * loadDistinctId(const char *token);
        
        static const char * getDeviceID();

        static void updateSuperproperty(const char *token, const char *superproperty);
        static const char *loadSuperproperty(const char *token);
        static void handleTECallback(int code,const string &str);
    };

#endif


    class ta_cpp_helper {
    public:
        static string getEventID();
        static string getDeviceID();
        static void printSDKLog(const string &log);
        static void printSDKLog(TDLogLevel level, const string &log);
        static void updateAccount(const char *token, const char *accountId, const char *path = "");
        static void updateDistinctId(const char *token, const char *distinctId, const char *path = "");

        static string loadAccount(const char *token, const char *path = "");
        static string loadDistinctId(const char *token, const char *path = "");

        static void updateSuperProperty(const char *token, const char *superproperty, const char *path = "");
        static string loadSuperProperty(const char *token, const char *path = "");
        static void handleTECallback(int code,const string &str);
        static int flush_interval;
        static int flush_bulk_size;
        static int mini_database_limit;
        static int64_t data_expression;

        static bool isStringOnlySpaces(const string& str);
    };
}
