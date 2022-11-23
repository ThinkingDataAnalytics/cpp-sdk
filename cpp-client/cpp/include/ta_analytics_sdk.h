
#pragma once
#ifndef SENSORS_ANALYTICS_SDK_H_
#define SENSORS_ANALYTICS_SDK_H_

#include <cstring>
#include <ctime>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "ta_json_object.h"

#define TD_LIB_VERSION "1.3.0"

#define TD_LIB_NAME "Cpp"

#ifdef _MSC_VER
#if _MSC_VER >= 1600
#include <cstdint>
#else
typedef __int32 int32_t;
typedef __int64 int64_t;
#endif
#elif __GNUC__ >= 3
#include <cstdint>
#endif

namespace thinkingdata {

using namespace std;

class ThinkingAnalyticsAPI;
class TAHttpSend;
class TDJSONObject;
class TDFirstEvent;
class TDUpdatableEvent;
class TDOverWritableEvent;
class TASqliteDataQueue;

class ThinkingAnalyticsAPI {
public:

    static bool Init(const string &server_url,
                     const string &distinct_id);
    
    static void EnableLog(bool enable);
    
    static void Login(const string &login_id);
    
    static void LogOut();
    
    static void Identify(const string &distinct_id);

    static void SetSuperProperty(const TDJSONObject &properties);

    static void ClearSuperProperty();

    static void Track(const string &event_name);
    
    static void Track(const string &event_name, const TDJSONObject &properties);
    
    static void Track(TDFirstEvent* event);
    
    static void Track(TDUpdatableEvent* event);
    
    static void Track(TDOverWritableEvent* event);
    
    static void UserSet(const TDJSONObject &properties);
    
    static void UserSetOnce(const TDJSONObject &properties);
    
    static void UserAdd(const TDJSONObject &properties);
    
    static void UserAppend(const TDJSONObject &properties);
    
    static void UserDelete();
    
    static void UserUnset(string propertyName);

    static void Flush();

    static string DistinctID();
    static string StagingFilePath();
    static bool IsEnableLog();
    
    ~ThinkingAnalyticsAPI();
    
private:
    ThinkingAnalyticsAPI(const string &server_url,
                         const string &appid);
    
    ThinkingAnalyticsAPI(const ThinkingAnalyticsAPI &);
    
    ThinkingAnalyticsAPI &operator=(const ThinkingAnalyticsAPI &);
    
    bool AddEvent(const string &action_type, const string &event_name,
                  const TDJSONObject &properties,
                  const string &firstCheckID = "",
                  const string &eventID = "");
    
    void AddUser(string eventType, const TDJSONObject &properties);

    void InnerFlush();
    
    static ThinkingAnalyticsAPI *instance_;
    
    string appid_;
    string server_url_;
    string account_id_;
    string distinct_id_;
    string staging_file_path_;
    TAHttpSend *httpSend_;
    TASqliteDataQueue *m_sqlite;
    TDJSONObject *m_superProperties;
    bool enableLog;
};

enum EventType {
    FIRST=1,
    UPDATABLE=2,
    OVERWRITABLE=3
};


class ThinkingAnalyticsEvent
{
public:
    EventType mType;
    string mEventName;
    TDJSONObject mProperties;
    string mExtraId;
    ThinkingAnalyticsEvent(string eventName,TDJSONObject properties);
};
class TDFirstEvent: public ThinkingAnalyticsEvent
{
public:
    TDFirstEvent(string eventName,TDJSONObject properties);
    void setFirstCheckId(string firstCheckId);

};
class TDUpdatableEvent: public ThinkingAnalyticsEvent
{
public:
    TDUpdatableEvent(string eventName,TDJSONObject properties,string eventId);
};
class TDOverWritableEvent: public ThinkingAnalyticsEvent
{
public:
    TDOverWritableEvent(string eventName,TDJSONObject properties,string eventId);
};

}

#endif
