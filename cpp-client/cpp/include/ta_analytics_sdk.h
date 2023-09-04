
#pragma once
#if defined(_WIN32)
#define THINKINGDATA_API __declspec(dllexport)
#endif
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
#define TD_LIB_VERSION "1.3.7.1"

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

#if defined(_WIN32)
class THINKINGDATA_API ThinkingAnalyticsAPI;
#else
class ThinkingAnalyticsAPI;
#endif
class TDConfig;
class TAHttpSend;
class TDJSONObject;
class TDFirstEvent;
class TDUpdatableEvent;
class TDOverWritableEvent;
class TASqliteDataQueue;
class TDTimeCalibrated;

enum TALogType {
    LOGNONE = 1,
    LOGCONSOLE = 2,
    LOGTXT = 3
};

class ThinkingAnalyticsAPI {
public:

    /**
     * After the SDK initialization is complete, the saved instance can be obtained through this app
     */
    static bool Init(const string &server_url,
                     const string &appid);

    static bool Init(TDConfig &config);

    // ThinkingAnalyticsAPI Destructor
    static void Unint();
    static void UnInit();
    
    static void EnableLog(bool enable);

    static void EnableLogType(TALogType type);

    /**
     * Set the account ID. Each setting overrides the previous value. Login events will not be uploaded.
     *
     * @param loginId account ID
     */
    static void Login(const string &login_id);

    /**
     * Clearing the account ID will not upload user logout events.
     */
    static void LogOut();

    /**
     * Set the distinct ID to replace the default UUID distinct ID.
     *
     * @param identify distinct ID
     */
    static void Identify(const string &distinct_id);

    /**
     * Set the public event attribute, which will be included in every event uploaded after that. The public event properties are saved without setting them each time.
     *
     * @param superProperties public event attribute
     */
    static void SetSuperProperty(const TDJSONObject &properties);

    /**
     *  Clear all public event attributes.
     */
    static void ClearSuperProperty();

    /**
     * track a event
     *
     * @param eventName event name
     */
    static void Track(const string &event_name);

    /**
     * track a event
     *
     * @param eventName event name
     * @param properties event properties
     */
    static void Track(const string &event_name, const TDJSONObject &properties);

    /**
    * The first event refers to the ID of a device or other dimension, which will only be recorded once.
    */
    static void Track(TDFirstEvent* event);

    /**
    * You can implement the requirement to modify event data in a specific scenario through updatable events. Updatable events need to specify an ID that identifies the event and pass it in when the updatable event object is created.
    */
    static void Track(TDUpdatableEvent* event);
    /**
    * Rewritable events will completely cover historical data with the latest data, which is equivalent to deleting the previous data and storing the latest data in effect.
    */
    static void Track(TDOverWritableEvent* event);

    /**
     * Sets the user property, replacing the original value with the new value if the property already exists.
     *
     * @param property user property
     */
    static void UserSet(const TDJSONObject &properties);

    /**
     *  Sets a single user attribute, ignoring the new attribute value if the attribute already exists.
     *
     * @param property user property
     */
    static void UserSetOnce(const TDJSONObject &properties);

    /**
     * Adds the numeric type user attributes.
     *
     * @param property user property
     */
    static void UserAdd(const TDJSONObject &properties);

    /**
     * Append a user attribute of the List type.
     *
     * @param property user property
     */
    static void UserAppend(const TDJSONObject &properties);

    /**
     * Delete the user attributes,This operation is not reversible and should be performed with caution.
     */
    static void UserDelete();

    /**
     * Reset user properties.
     *
     * @param properties user properties
     */
    static void UserUnset(string propertyName);

    /**
     * Empty the cache queue. When this api is called, the data in the current cache queue will attempt to be reported.
     * If the report succeeds, local cache data will be deleted.
     */
    static void Flush();

    static void CalibrateTime(int64_t &timestamp);

    static string DistinctID();
    static string StagingFilePath();

    static void registerTECallback(void(*p)(int,const string&));

    static vector<void(*)(int,const string&)> getTECallback();

    ~ThinkingAnalyticsAPI();

    static ThinkingAnalyticsAPI* instance_;

private:
    /**
     * After the SDK initialization is complete, the saved instance can be obtained through this api
     */
    ThinkingAnalyticsAPI(const string &server_url,
                         const string &appid);

    ThinkingAnalyticsAPI(const ThinkingAnalyticsAPI &);

    //ThinkingAnalyticsAPI &operator=(const ThinkingAnalyticsAPI &);

    bool AddEvent(const string &action_type, const string &event_name,
                  const TDJSONObject &properties,
                  const string &firstCheckID = "",
                  const string &eventID = "");

    void AddUser(string eventType, const TDJSONObject &properties);

    void InnerFlush();

    static void fetchRemoteConfigCallback(bool calibrateTime);

    string appid_;
    string server_url_;
    string account_id_;
    string distinct_id_;
    string device_id_;
    string staging_file_path_;
    TAHttpSend *httpSend_;
    TASqliteDataQueue *m_sqlite;
    TDTimeCalibrated *timeCalibrated;
    TDJSONObject m_superProperties;
    vector<void(*)(int,const string&)> funcs;
};

enum EventType {
    FIRST=1,
    UPDATABLE=2,
    OVERWRITABLE=3
};

class TDConfig{
public:
    string appid;
    string server_url;
    bool enableEncrypt = false;
    bool enableAutoCalibrated = false;
    void EnableEncrypt(int version,const string &publicKey);
    int version;
    string publicKey;
    ~TDConfig();
};

class ThinkingAnalyticsEvent
{
public:
    EventType mType;
    string mEventName;
    TDJSONObject mProperties;
    string mExtraId;
    ThinkingAnalyticsEvent(const string &eventName, const TDJSONObject &properties);
};
class TDFirstEvent: public ThinkingAnalyticsEvent
{
public:
    TDFirstEvent(const string &eventName,const TDJSONObject & properties);
    void setFirstCheckId(const string & firstCheckId);

};
class TDUpdatableEvent: public ThinkingAnalyticsEvent
{
public:
    TDUpdatableEvent(const string & eventName,const TDJSONObject & properties, const string & eventId);
};
class TDOverWritableEvent: public ThinkingAnalyticsEvent
{
public:
    TDOverWritableEvent(const string &eventName,const TDJSONObject & properties, const string & eventId);
};

}

#endif
