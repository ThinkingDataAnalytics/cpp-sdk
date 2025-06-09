
#include "ta_cpp_helper.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include "ta_cpp_utils.h"
#if defined(_WIN32)
#include <windows.h>
#else
#include <uuid/uuid.h>
#endif
#include <mutex>


namespace thinkingdata {

    using namespace std;
    mutex ta_log_mutex;

    int ta_cpp_helper::flush_bulk_size = 30;
    int ta_cpp_helper::flush_interval = 15;
    int ta_cpp_helper::mini_database_limit = 10000;
    int64_t ta_cpp_helper::data_expression = 10*24*60*60;

    string ta_cpp_helper::getEventID() {
#ifdef _WIN32

        string uuid;
        uuid.resize(36);

        char characterSet[] = "0123456789abcdefghijklmnopqrstuvwxyz";

        // 8 characters
        for (int i = 0; i < 8; i++)
            uuid[i] = characterSet[rand() % 36];

        // 4 characters
        for (int i = 9; i < 13; i++)
            uuid[i] = characterSet[rand() % 36];

        // 4 characters
        for (int i = 14; i < 18; i++)
            uuid[i] = characterSet[rand() % 36];

        // 4 characters
        for (int i = 19; i < 23; i++)
            uuid[i] = characterSet[rand() % 36];

        // 12 characters
        for (int i = 24; i < 36; i++)
            uuid[i] = characterSet[rand() % 36];

        // Separators
        uuid[8] = '-';
        uuid[13] = '-';
        uuid[18] = '-';
        uuid[23] = '-';

 
        return uuid;
#else
        std::string guid("");
        uuid_t uuid;
        char str[50] = {};
        uuid_generate(uuid);
        uuid_unparse(uuid, str);
        guid.assign(str);
        return guid;
#endif
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

    void ta_cpp_helper::printSDKLog(TDLogLevel level, const string &log) {
        string logPrefix = "[ThinkingData] ";
        string detailLog;
        switch (level) {
            case TDLogLevel::TDDEBUG:
                detailLog = logPrefix + "Debug: " + log;
                break;
            case TDLogLevel::TDINFO:
                detailLog = logPrefix + "Info: " + log;
                break;
            case TDLogLevel::TDERROR:
                detailLog = logPrefix + "Error: " + log;
                break;
            default:
                detailLog = logPrefix + "Unknown: " + log;
                break;
        }
        printSDKLog(detailLog);
    }

    void ta_cpp_helper::printSDKLog(const string &log) {
        TALogType type = TAEnableLog::getTALogType();
        if (type == LOGCONSOLE) {
            cout << log << endl;
        }else if (type == LOGTXT) {
            ta_log_mutex.lock();
            std::ofstream LogFile;
            LogFile.open("ta_event_log.txt", std::ios::app);
            if (LogFile.is_open()) {
                const std::streampos max_size = 1024*1024*10;
                LogFile.seekp(0, std::ios::end);
                if (LogFile.tellp() < max_size) {
                #if defined(_WIN32)
                    if (CheckUtf8Valid(log.c_str())) {
                        char* str = U2G(log.c_str());
                        string tmpStr = string(str);
                        LogFile << tmpStr << "\n";
                        delete str;
                    }
                    else {
                        LogFile << log << "\n";
                    }
                #elif defined(__APPLE__)
                    LogFile << log << "\n";
                #endif
                }
                LogFile.close();
            }
            ta_log_mutex.unlock();
        }
    }

    void ta_cpp_helper::handleTECallback(int code, const string& str) {
        for (int i = 0; i < ThinkingAnalyticsAPI::getTECallback().size(); ++i) {
            ThinkingAnalyticsAPI::getTECallback()[i](code,str);
        }
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

    bool ta_cpp_helper::isStringOnlySpaces(const std::string &str) {
        for (char c : str) {
            if (!std::isspace(c)) {
                return false;
            }
        }
        return true;
    }
}