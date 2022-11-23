
#include "ta_cpp_helper.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace thinkingdata {

    using namespace std;

    string ta_cpp_helper::getEventID() {
        char str_uuid[80];
        static bool hassrand;
        if (hassrand != true) {
            srand(time(NULL));
            hassrand = true;
        }
        snprintf(str_uuid, sizeof(str_uuid),
                 "%x%x-%x-%x-%x-%x%x%x",
                 rand(),
                 rand(),
                 rand(),
                 ((rand() & 0x0fff) | 0x4000),
                 rand() % 0x3fff + 0x8000,
                 rand(),
                 rand(),
                 rand());
        return string(str_uuid);
    }
    
    string ta_cpp_helper::getDeviceID() {
        
    #if defined(_WIN32)
        std::string ret;
        char value[64];
        DWORD size = _countof(value);
        DWORD type = REG_SZ;
        HKEY key;
        LONG retKey = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &key);
        LONG retVal = ::RegQueryValueExA(key, "MachineGuid", nullptr, &type, (LPBYTE)value, &size);
        if (retKey == ERROR_SUCCESS && retVal == ERROR_SUCCESS) {
            ret = value;
        }
        ::RegCloseKey(key);
        return ret;
    #elif defined(__APPLE__)
        return  ta_mac_tool::getDeviceID();
    #endif
        
        return "";
    }
    
    
    void ta_cpp_helper::updateAccount(const char *token, const char *accountId, const char *path) {
    #if defined(_WIN32)
        string path1 = string(path) + string(token) + "_accountId_tag";
        std::ofstream staging_ofs(path1.c_str(), std::ofstream::out);
        staging_ofs << accountId << std::endl;
    #elif defined(__APPLE__)
        ta_mac_tool::updateAccount(token, accountId);
    #endif
    }
    
    void ta_cpp_helper::updateDistinctId(const char *token, const char *distinctId, const char *path) {
    #if defined(_WIN32)
        string path1 = string(path) + string(token) + "_distinctId_tag";
        std::ofstream staging_ofs(path1.c_str(), std::ofstream::out);
        staging_ofs << distinctId << std::endl;
        staging_ofs.close();
    #elif defined(__APPLE__)
        ta_mac_tool::updateDistinctId(token, distinctId);
    #endif
    }
    
    string ta_cpp_helper::loadAccount(const char *token, const char *path) {
    #if defined(_WIN32)
        string path1 = string(path) + string(token) + "_accountId_tag";
        std::ifstream staging_ifs((path1), std::ofstream::in);
        string line;
        std::getline(staging_ifs, line);
        return line;
    #elif defined(__APPLE__)
        const char *accountID = ta_mac_tool::loadAccount(token);
        if (accountID == NULL) {
            return "";
        } else {
            return accountID;
        }
    #endif
    }
    
    string ta_cpp_helper::loadDistinctId(const char *token, const char *path) {
    #if defined(_WIN32)
        string path1 = string(path) + string(token) + "_distinctId_tag";
        std::ifstream staging_ifs((path1), std::ofstream::in);
        string line;
        std::getline(staging_ifs, line);
        return line;
    #elif defined(__APPLE__)
        const char *distincID = ta_mac_tool::loadDistinctId(token);
        if (distincID == NULL) {
            return "";
        } else {
            return distincID;
        }
    #endif
    }
    
    void ta_cpp_helper::updateSuperProperty(const char *token, const char *superproperty, const char *path)
    {
    #if defined(_WIN32)
        string path1 = string(path) + string(token) + "_superproperty_tag";
        std::ofstream staging_ofs(path1.c_str(), std::ofstream::out);
        staging_ofs << superproperty << std::endl;
        staging_ofs.close();
    #elif defined(__APPLE__)
        ta_mac_tool::updateSuperproperty(token, superproperty);
    #endif
    }
    string ta_cpp_helper::loadSuperProperty(const char *token, const char *path)
    {
    #if defined(_WIN32)
        string path1 = string(path) + string(token) + "_superproperty_tag";
        std::ifstream staging_ifs((path1), std::ofstream::in);
        string line;
        std::getline(staging_ifs, line);
        return line;
    #elif defined(__APPLE__)
        const char *string = ta_mac_tool::loadSuperproperty(token);
        if (string == NULL) {
            return "";
        } else {
            return string;
        }
    #endif
    }

}