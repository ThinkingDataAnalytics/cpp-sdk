
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

#define TD_LIB_VERSION "1.0.0"

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

class DefaultConsumer;

class PropertiesNode;

namespace utils {

class TDJSONObject;
class TDFirstEvent;
class TDUpdatableEvent;
class TDOverWritableEvent;
}


class ThinkingAnalyticsAPI {
public:

    static bool Init(const string &server_url,
                     const string &distinct_id,
                     const string &data_file_path = "");
    
    static void EnableLog(bool enable);
    
    static void Login(const string &login_id);
    
    static void LogOut();
    
    static void Identify(const string &distinct_id);
    
    static void Track(const string &event_name);
    
    static void Track(const string &event_name, const PropertiesNode &properties);
    
    static void Track(utils::TDFirstEvent* event);
    
    static void Track(utils::TDUpdatableEvent* event);
    
    static void Track(utils::TDOverWritableEvent* event);
    
    static void UserSet(const utils::TDJSONObject &properties);
    
    static void UserSetOnce(const utils::TDJSONObject &properties);
    
    static void UserAdd(const utils::TDJSONObject &properties);
    
    static void UserAppend(const utils::TDJSONObject &properties);
    
    static void UserDelete();
    
    static void UserUnset(string propertyName);

    static string DistinctID();
    static string StagingFilePath();
    static bool IsEnableLog();
    
    ~ThinkingAnalyticsAPI();
    
private:
    ThinkingAnalyticsAPI(const string &server_url,
                         const string &appid);
    
    ThinkingAnalyticsAPI(const string &server_url,
                         const string &appid,
                         const string &data_file_path);
    
    ThinkingAnalyticsAPI(const ThinkingAnalyticsAPI &);
    
    ThinkingAnalyticsAPI &operator=(const ThinkingAnalyticsAPI &);
    
    bool AddEvent(const string &action_type, const string &event_name,
                  const utils::TDJSONObject &properties,
                  const string &firstCheckID = "",
                  const string &eventID = "");
    
    void AddUser(string eventType, const utils::TDJSONObject &properties);
    
    static ThinkingAnalyticsAPI *instance_;
    
    string appid_;
    string server_url_;
    string account_id_;
    string distinct_id_;
    string staging_file_path_;
    DefaultConsumer *consumer_;
};

namespace utils {

class TDJSONObject {
public:
    void SetNumber(const string &property_name, int32_t value);
    
    void SetNumber(const string &property_name, int64_t value);
    
    void SetNumber(const string &property_name, double value);
    
    void SetString(const string &property_name, const string &value);
    
    void SetString(const string &property_name, const char *value);
    
    void SetBool(const string &property_name, bool value);
    
    void SetList(const string &property_name, const std::vector<string> &value);
    
    void SetList(const string &property_name, const std::vector<TDJSONObject> &value);
    
    void SetDateTime(const string &property_name, time_t seconds,
                     int milliseconds);
    
    // 字符串格式需要是: 2018-09-07 16:30:22.567
    void SetDateTime(const string &property_name, const string &value);
    
    void Clear();
    
    virtual void SetObject(const string &property_name, const TDJSONObject &value);
    
    static string ToJson(const TDJSONObject &node);
    
    TDJSONObject();
    
    class ValueNode;
    
    std::map<string, ValueNode> properties_map_;
    
    static void DumpNode(const TDJSONObject &node, string *buffer);
    
    void MergeFrom(const TDJSONObject &another_node);
    
    friend class ::thinkingdata::ThinkingAnalyticsAPI;
    
    enum ValueNodeType {
        NUMBER,
        INT,
        STRING,
        LIST,
        DATETIME,
        BOOL,
        OBJECT,
        OBJECTS,
        UNKNOWN,
    };
};

class TDJSONObject::ValueNode {
public:
    ValueNode() : node_type_(UNKNOWN) {}
    
    explicit ValueNode(double value);
    
    explicit ValueNode(int64_t value);
    
    explicit ValueNode(const string &value);
    
    explicit ValueNode(bool value);
    
    explicit ValueNode(const TDJSONObject &value);
    
    explicit ValueNode(const std::vector<string> &value);
    
    explicit ValueNode(const std::vector<TDJSONObject> &value);
    
    ValueNode(time_t seconds, int milliseconds);
    
    static void ToStr(const ValueNode &node, string *buffer);
    
private:
    static void DumpString(const string &value, string *buffer);
    
    static void DumpList(const std::vector<string> &value, string *buffer);
    
    static void DumpList(const std::vector<TDJSONObject> &value, string *buffer);
    
    static void DumpDateTime(const time_t &seconds, int milliseconds,
                             string *buffer);
    
    static void DumpNumber(double value, string *buffer);
    
    static void DumpNumber(int64_t value, string *buffer);
    
    friend class ::thinkingdata::ThinkingAnalyticsAPI;
    
    ValueNodeType node_type_;
    
    union UnionValue {
        double number_value;
        bool bool_value;
        struct {
            std::time_t seconds;
            int milliseconds;
        } date_time_value;
        int64_t int_value;
        
        UnionValue() { memset(this, 0, sizeof(UnionValue)); }
    } value_;
    
    string string_data_;
    std::vector<string> list_data_;
    std::vector<TDJSONObject> list_obj_;
    TDJSONObject object_data_;
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



class PropertiesNode : public utils::TDJSONObject {

    friend class ThinkingAnalyticsAPI;
    
    void SetObject(const string &property_name, const TDJSONObject &value);
};

}

#endif
