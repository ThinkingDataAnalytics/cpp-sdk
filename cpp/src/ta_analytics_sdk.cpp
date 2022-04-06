
#include "ta_analytics_sdk.h"
#include "ta_cpp_network.h"

#include <curl/curl.h>
#include <zlib.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>
#include <sys/timeb.h>

#if defined(_WIN32)
#include <windows.h>
#include <iostream>
#include <fstream>
#else

#include <pthread.h>
#include <sys/time.h>

#endif

#include "ta_cpp_helper.h"

const static std::string TD_EVENT_TYPE                 = "track";
const static std::string TD_EVENT_TYPE_TRACK_FIRST     = "track_first";
const static std::string TD_EVENT_TYPE_TRACK_UPDATE    = "track_update";
const static std::string TD_EVENT_TYPE_TRACK_OVERWRITE = "track_overwrite";

const static std::string TD_EVENT_TYPE_USER_DEL        = "user_del";
const static std::string TD_EVENT_TYPE_USER_ADD        = "user_add";
const static std::string TD_EVENT_TYPE_USER_SET        = "user_set";
const static std::string TD_EVENT_TYPE_USER_SETONCE    = "user_setOnce";
const static std::string TD_EVENT_TYPE_USER_UNSET      = "user_unset";
const static std::string TD_EVENT_TYPE_USER_APPEND     = "user_append";

namespace thinkingdata {

namespace utils {

int64_t currentTimestamp() {
    int64_t current_timestamp;
#if defined(_WIN32)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    const int64_t kUnixTimeStart = 0x019DB1DED53E8000L;
    const int64_t kTicksPerMillisecond = 10000;
    LARGE_INTEGER li;
    li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    current_timestamp = (li.QuadPart - kUnixTimeStart) / kTicksPerMillisecond;
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    current_timestamp = (long long) now.tv_sec * 1000 + (long) (now.tv_usec / 1000);
#endif
    return current_timestamp;
}

int64_t generateTrackId() {
    string first = std::to_string(rand() % 99 + 100);
    string first_sub = first.substr(first.length() - 2, 2);
    
    string second = std::to_string(rand() % 999 + 1000);
    string second_sub = second.substr(second.length() - 3, 3);
    
    string timestamp = std::to_string(currentTimestamp());
    string timestamp_sub = timestamp.substr(timestamp.length() - 4, 4);
    
    return std::stoi(first_sub + second_sub + timestamp_sub);
}

string urlEncode(const string &data) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (std::string::size_type i = 0; i < data.size(); ++i) {
        unsigned char c = data[i];
        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }
        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }
    return escaped.str();
}

unsigned char decimalFromHex(unsigned char x) {
    unsigned char y = '\0';
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    return y;
}

string urlDecode(const string &data)
{
    std::string result = "";
    size_t length = data.length();
    for (size_t i = 0; i < length; i++)
    {
        if (data[i] == '+') result += ' ';
        else if (data[i] == '%') {
            unsigned char high = decimalFromHex((unsigned char)data[++i]);
            unsigned char low = decimalFromHex((unsigned char)data[++i]);
            result += high * 16 + low;
        }
        else result += data[i];
    }
    return result;
}

void TDJSONObject::SetNumber(const string &property_name, double value) {
    properties_map_[property_name] = ValueNode(value);
}

void TDJSONObject::SetNumber(const string &property_name, int32_t value) {
    properties_map_[property_name] = ValueNode(static_cast<int64_t>(value));
}

void TDJSONObject::SetNumber(const string &property_name, int64_t value) {
    properties_map_[property_name] = ValueNode(value);
}

static const size_t kStringPropertyValueMaxLength = 8192;

bool CheckUtf8Valid(const string &str) {
    const unsigned char *bytes = (const unsigned char *) str.data();
    const unsigned char *begin = bytes;
    while (bytes - begin < (int)str.length()) {
        if ((bytes[0] == 0x09 || bytes[0] == 0x0A || bytes[0] == 0x0D ||
             (0x20 <= bytes[0] && bytes[0] <= 0x7E))) {
            bytes += 1;
            continue;
        }
        if (((0xC2 <= bytes[0] && bytes[0] <= 0xDF)
             && (0x80 <= bytes[1] && bytes[1] <= 0xBF))) {
            bytes += 2;
            continue;
        }
        if ((bytes[0] == 0xE0 && (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
             (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
            (((0xE1 <= bytes[0] && bytes[0] <= 0xEC) || bytes[0] == 0xEE
              || bytes[0] == 0xEF) &&
             (0x80 <= bytes[1] && bytes[1] <= 0xBF)
             && (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
            (bytes[0] == 0xED && (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
             (0x80 <= bytes[2] && bytes[2] <= 0xBF))) {
            bytes += 3;
            continue;
        }
        if ((bytes[0] == 0xF0 && (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
             (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
             (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
            ((0xF1 <= bytes[0] && bytes[0] <= 0xF3)
             && (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
             (0x80 <= bytes[2] && bytes[2] <= 0xBF)
             && (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
            (bytes[0] == 0xF4 && (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
             (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
             (0x80 <= bytes[3] && bytes[3] <= 0xBF))) {
            bytes += 4;
            continue;
        }
        return false;
    }
    return bytes - begin == str.length();
}

void TDJSONObject::SetString(const string &property_name, const string &value) {
    if (value.length() > kStringPropertyValueMaxLength) {
        std::cerr << "String property '" << property_name
        << "' is too long, value: " << value << std::endl;
        return;
    }
    if (!CheckUtf8Valid(value)) {
        std::cerr << "String property '" << property_name
        << "' is not valid UTF-8 string, value: " << value
        << std::endl;
        return;
    }
    properties_map_[property_name] = ValueNode(value);
}

void TDJSONObject::SetString(const string &property_name, const char *value) {
    SetString(property_name, string(value));
}

void TDJSONObject::SetBool(const string &property_name, bool value) {
    properties_map_[property_name] = ValueNode(value);
}

void
TDJSONObject::SetObject(const string &property_name, const TDJSONObject &value) {
    properties_map_[property_name] = ValueNode(value);
}

void TDJSONObject::SetList(const string &property_name,
                           const std::vector<string> &value) {
    properties_map_[property_name] = ValueNode(value);
}


void TDJSONObject::SetList(const string &property_name,
                           const std::vector<TDJSONObject> &value) {
    properties_map_[property_name] = ValueNode(value);
}

void TDJSONObject::SetDateTime(const string &property_name,
                               const time_t seconds,
                               int milliseconds) {
    properties_map_[property_name] = ValueNode(seconds, milliseconds);
}

void utils::TDJSONObject::SetDateTime(const string &property_name,
                                      const string &value) {
    properties_map_[property_name] = ValueNode(value);
}

void TDJSONObject::Clear() {
    properties_map_.clear();
}

void TDJSONObject::DumpNode(const TDJSONObject &node, string *buffer) {
    *buffer += '{';
    bool first = true;
    
    for (std::map<string, ValueNode>::const_iterator
         iterator = node.properties_map_.begin();
         iterator != node.properties_map_.end(); ++iterator) {
        if (first) {
            first = false;
        } else {
            *buffer += ',';
        }
        *buffer += '"' + iterator->first + "\":";
        ValueNode::ToStr(iterator->second, buffer);
    }
    *buffer += '}';
}

void TDJSONObject::ValueNode::DumpString(const string &value, string *buffer) {
    *buffer += '"';
    for (std::string::size_type i = 0; i < value.length(); ++i) {
        char c = value[i];
        switch (c) {
            case '"':
                *buffer += "\\\"";
                break;
            case '\\':
                *buffer += "\\\\";
                break;
            case '\b':
                *buffer += "\\b";
                break;
            case '\f':
                *buffer += "\\f";
                break;
            case '\n':
                *buffer += "\\n";
                break;
            case '\r':
                *buffer += "\\r";
                break;
            case '\t':
                *buffer += "\\t";
                break;
            default:
                *buffer += c;
                break;
        }
    }
    *buffer += '"';
}

void TDJSONObject::ValueNode::DumpList(const std::vector<string> &value,
                                       string *buffer) {
    *buffer += '[';
    bool first = true;
    for (std::vector<string>::const_iterator iterator = value.begin();
         iterator != value.end(); ++iterator) {
        if (first) {
            first = false;
        } else {
            *buffer += ',';
        }
        DumpString(*iterator, buffer);
    }
    *buffer += ']';
}

void TDJSONObject::ValueNode::DumpList(const std::vector<TDJSONObject> &value, string *buffer) {
    *buffer += '[';
    bool first = true;
    for (std::vector<TDJSONObject>::const_iterator iterator = value.begin(); iterator != value.end(); ++iterator) {
        if (first) {
            first = false;
        } else {
            *buffer += ',';
        }
        DumpNode(*iterator, buffer);
    }
    *buffer += ']';
}


#if defined(__linux__)
#define TD_SDK_LOCALTIME(seconds, now) localtime_r((seconds), (now))
#elif defined(__APPLE__)
#define TD_SDK_LOCALTIME(seconds, now) localtime_r((seconds), (now))
#elif defined(_WIN32)
#define TD_SDK_LOCALTIME(seconds, now) localtime_s((now), (seconds))
#define snprintf sprintf_s
#endif

void TDJSONObject::ValueNode::DumpDateTime(const time_t &seconds,
                                           int milliseconds,
                                           string *buffer) {
    struct tm tm = {};
    TD_SDK_LOCALTIME(&seconds, &tm);
    char buff[64];
    snprintf(buff, sizeof(buff), "\"%04d-%02d-%02d %02d:%02d:%02d.%03d\"",
             tm.tm_year + 1900,
             tm.tm_mon + 1,
             tm.tm_mday,
             tm.tm_hour,
             tm.tm_min,
             tm.tm_sec,
             milliseconds);
    *buffer += buff;
}

string TDJSONObject::ToJson(const TDJSONObject &node) {
    string buffer;
    DumpNode(node, &buffer);
    return buffer;
}

void TDJSONObject::MergeFrom(const utils::TDJSONObject &another_node) {
    for (std::map<string, ValueNode>::const_iterator
         iterator = another_node.properties_map_.begin();
         iterator != another_node.properties_map_.end(); ++iterator) {
        properties_map_[iterator->first] = iterator->second;
    }
}

TDJSONObject::TDJSONObject() {}

utils::TDJSONObject::ValueNode::ValueNode(double value) : node_type_(NUMBER) {
    value_.number_value = value;
}

utils::TDJSONObject::ValueNode::ValueNode(int64_t value) : node_type_(INT) {
    value_.int_value = value;
}

utils::TDJSONObject::ValueNode::ValueNode(const string &value)
: node_type_(STRING),
string_data_(value) {}

utils::TDJSONObject::ValueNode::ValueNode(bool value) : node_type_(BOOL) {
    value_.bool_value = value;
}

utils::TDJSONObject::ValueNode::ValueNode(const utils::TDJSONObject &value)
: node_type_(OBJECT) {
    object_data_ = value;
}

utils::TDJSONObject::ValueNode::ValueNode(const std::vector<string> &value)
: node_type_(LIST),
list_data_(value) {}

TDJSONObject::ValueNode::ValueNode(const std::vector<TDJSONObject> &value)
: node_type_(OBJECTS),
list_obj_(value) {}

utils::TDJSONObject::ValueNode::ValueNode(time_t seconds, int milliseconds)
: node_type_(DATETIME) {
    value_.date_time_value.seconds = seconds;
    value_.date_time_value.milliseconds = milliseconds;
}

utils::ThinkingAnalyticsEvent::ThinkingAnalyticsEvent(string eventName, TDJSONObject properties) {
    this->mEventName = eventName;
    this->mProperties = properties;
}
utils::TDFirstEvent::TDFirstEvent(string eventName, TDJSONObject properties):ThinkingAnalyticsEvent(eventName,properties) {
    this->mType = FIRST;
    this->mExtraId = "";
}

void utils::TDFirstEvent::setFirstCheckId(string firstCheckId) {
    this->mExtraId = firstCheckId;
}
utils::TDUpdatableEvent::TDUpdatableEvent(string eventName, TDJSONObject properties, string eventId):ThinkingAnalyticsEvent(eventName,properties) {
    this->mExtraId = eventId;
    this->mType = UPDATABLE;
}
utils::TDOverWritableEvent::TDOverWritableEvent(string eventName, TDJSONObject properties, string eventId):ThinkingAnalyticsEvent(eventName,properties) {
    this->mExtraId = eventId;
    this->mType = OVERWRITABLE;
}



void
utils::TDJSONObject::ValueNode::ToStr(const utils::TDJSONObject::ValueNode &node,
                                      string *buffer) {
    switch (node.node_type_) {
        case NUMBER:
            DumpNumber(node.value_.number_value, buffer);
            break;
        case INT:
            DumpNumber(node.value_.int_value, buffer);
            break;
        case STRING:
            DumpString(node.string_data_, buffer);
            break;
        case LIST:
            DumpList(node.list_data_, buffer);
            break;
        case BOOL:
            *buffer += (node.value_.bool_value ? "true" : "false");
            break;
        case OBJECT:
            DumpNode(node.object_data_, buffer);
            break;
        case DATETIME:
            DumpDateTime(node.value_.date_time_value.seconds,
                         node.value_.date_time_value.milliseconds, buffer);
            break;
        case OBJECTS:
            DumpList(node.list_obj_, buffer);
            break;
        default:
            break;
    }
}

void TDJSONObject::ValueNode::DumpNumber(double value, string *buffer) {
    std::ostringstream buf;
    buf.imbue(locale("C"));
    buf << value;
    *buffer += buf.str();
}

void TDJSONObject::ValueNode::DumpNumber(int64_t value, string *buffer) {
    std::ostringstream buf;
    buf.imbue(locale("C"));
    buf << value;
    *buffer += buf.str();
}

}

class HttpSender {
public:
    HttpSender(const string &server_url,
                        const string &appid);
    
    bool send(const string &data);
    
private:
    static bool gzipString(const string &str,
                           string *out_string,
                           int compression_level);
    
    static bool encodeToRequestBody(const string &data,
                                    string *request_body);
    
    static string base64Encode(const string &data);
    
    friend class ThinkingAnalyticsAPI;
    
    static const int kRequestTimeoutSecond = 3;
    string server_url_;
    string appid_;
};

HttpSender::HttpSender(const string &server_url,
                       const string &appid)
:server_url_(server_url), appid_(appid){
    
    if (server_url_[std::strlen(server_url_.c_str()) - 1] == '/') {
        server_url_ = server_url_ + "sync";
    } else {
        server_url_ = server_url_ + "/sync";
    }
}

bool HttpSender::send(const string &data) {
    string request_body;
    if (!encodeToRequestBody(data, &request_body)) {
        return false;
    }
    
    Response response = Post(server_url_,
                             request_body,
                             kRequestTimeoutSecond);
    if (response.code_ != 200) {
        std::cerr << "ThinkingAnalytics SDK send failed: " << response.body_
        << std::endl;
        return false;
    }
    return true;
}

bool HttpSender::gzipString(const string &str,
                                string *out_string,
                                int compression_level = Z_BEST_COMPRESSION) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (deflateInit2(&zs, compression_level, Z_DEFLATED,
                     15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        std::cerr << "deflateInit2 failed while compressing." << std::endl;
        return false;
    }
    
    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(str.data()));
    zs.avail_in = static_cast<uInt>(str.size()); 
    
    int ret;
    char out_buffer[32768];

    do {
        zs.next_out = reinterpret_cast<Bytef *>(out_buffer);
        zs.avail_out = sizeof(out_buffer);
        
        ret = deflate(&zs, Z_FINISH);
        
        if (out_string->size() < zs.total_out) {
            out_string->append(out_buffer, zs.total_out - out_string->size());
        }
    } while (ret == Z_OK);
    
    deflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {  
        std::cerr << "Exception during zlib compression: (" << ret << ") " << zs.msg
        << std::endl;
        return false;
    }
    
    return true;
}

static const char kBase64Chars[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string HttpSender::base64Encode(const string &data) {
    const unsigned char
    *bytes_to_encode = reinterpret_cast<const unsigned char *>(data.data());
    size_t in_len = data.length();
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (in_len-- > 0) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; (i < 4); i++)
                ret += kBase64Chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i != 0) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        for (j = 0; (j < i + 1); j++)
            ret += kBase64Chars[char_array_4[j]];
        while ((i++ < 3))
            ret += '=';
    }
    return ret;
}

bool HttpSender::encodeToRequestBody(const string &data, string *request_body) {
    string compressed_data;
    if (!gzipString(data, &compressed_data)) {
        return false;
    }
    const string base64_encoded_data = base64Encode(compressed_data);
    *request_body = base64_encoded_data;
    return true;
}

void PropertiesNode::SetObject(const string &property_name,
                               const utils::TDJSONObject &value) {}

class DefaultConsumer {
public:
    DefaultConsumer(const string &server_url,
                    const string &appid,
                    const string &data_file_path);
    
    void Init();
    
    void Send(const utils::TDJSONObject &record);

    void EnableLog(bool enable_log);
    
    bool enable_log_;
    
    ~DefaultConsumer();
    
private:
    
    static const size_t kFlushAllBatchSize = 30;
    
#if defined(_WIN32)
#define TD_MUTEX CRITICAL_SECTION
#define TD_MUTEX_LOCK(mutex) EnterCriticalSection((mutex))
#define TD_MUTEX_UNLOCK(mutex) LeaveCriticalSection((mutex))
#define TD_MUTEX_INIT(mutex) InitializeCriticalSection((mutex))
#define TD_MUTEX_DESTROY(mutex) DeleteCriticalSection((mutex))
#else
#define TD_MUTEX pthread_mutex_t
#define TD_MUTEX_LOCK(mutex) pthread_mutex_lock((mutex))
#define TD_MUTEX_UNLOCK(mutex) pthread_mutex_unlock((mutex))
#define TD_MUTEX_INIT(mutex) \
do { \
pthread_mutexattr_t mutex_attr; \
pthread_mutexattr_init(&mutex_attr); \
pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE); \
pthread_mutex_init((mutex), &mutex_attr); \
} while(0)
#define TD_MUTEX_DESTROY(mutex) pthread_mutex_destroy((mutex))
#endif
    
    TD_MUTEX records_mutex_; 
    TD_MUTEX sending_mutex_;
    
    class LockGuard {
    public:
        LockGuard(TD_MUTEX *mutex) : mutex_(mutex) {
            TD_MUTEX_LOCK(mutex_);
        }
        
        ~LockGuard() {
            TD_MUTEX_UNLOCK(mutex_);
        }
        
    private:
        TD_MUTEX *mutex_;
    };
    
    std::deque<string> records_;
    string data_file_path_;
    HttpSender *sender_;
};

DefaultConsumer::DefaultConsumer(const string &server_url,
                                 const string &appid,
                                 const string &data_file_path)
: enable_log_(false),
sender_(new HttpSender(server_url, appid)) {}

void DefaultConsumer::Send(const utils::TDJSONObject &record) {
    const string json_record = utils::TDJSONObject::ToJson(record);
    std::stringstream buffer;
    if (enable_log_) {
        std::cout << "record : " + json_record << std::endl;
    }
    
    std::vector<std::pair<string,string> > http_headers;
    bool send_result = sender_->send(json_record);
}

void DefaultConsumer::EnableLog(bool enable_log) {
    enable_log_ = enable_log;
}

DefaultConsumer::~DefaultConsumer() {
}


void DefaultConsumer::Init() {
    TD_MUTEX_INIT(&records_mutex_);
    TD_MUTEX_INIT(&sending_mutex_);
}

ThinkingAnalyticsAPI *ThinkingAnalyticsAPI::instance_ = NULL;

#define RETURN_IF_ERROR(stmt) do { if (!stmt) return false; } while (false)

bool ThinkingAnalyticsAPI::Init(const std::string &server_url,
                                const std::string &appid,
                                const std::string &data_file_path) {
    if (!instance_) {
        
        instance_ = new ThinkingAnalyticsAPI(server_url, data_file_path);
        instance_->consumer_->Init();
        srand((unsigned)time(NULL));
        
        instance_->distinct_id_ = ta_cpp_helper::getDeviceID();
        instance_->account_id_ = "";
        
        // 恢复distinctid、accountid
        string accountid = ta_cpp_helper::loadAccount(appid.c_str(), data_file_path.c_str());
        string distinctid = ta_cpp_helper::loadDistinctId(appid.c_str(), data_file_path.c_str());
        if (accountid.length() != 0) {
            instance_->account_id_ = accountid;
        }
        if (distinctid.size() != 0) {
            instance_->distinct_id_ = distinctid;
        }
        
        instance_->appid_ = appid;
        instance_->server_url_ = server_url;
        
    }
    return true;
}

void ThinkingAnalyticsAPI::EnableLog(bool enable) {
    instance_->consumer_->EnableLog(enable);

}

void ThinkingAnalyticsAPI::AddUser(string eventType, const utils::TDJSONObject &properties)
{
    AddEvent(eventType, "", properties, "", "");
}


bool ThinkingAnalyticsAPI::AddEvent(const string &action_type,
                                    const string &event_name,
                                    const utils::TDJSONObject &properties,
                                    const string &firstCheckID,
                                    const string &eventID) {

    utils::TDJSONObject flushDic;
    utils::TDJSONObject finalDic;
    utils::TDJSONObject propertyDic;
    
    string first_check_id = firstCheckID;
    string eventType = action_type;
    string event_id = eventID;
    
    bool isTrackEvent = (eventType == TD_EVENT_TYPE || eventType == TD_EVENT_TYPE_TRACK_FIRST || eventType == TD_EVENT_TYPE_TRACK_UPDATE || eventType == TD_EVENT_TYPE_TRACK_OVERWRITE);
    

    if (isTrackEvent && eventType == TD_EVENT_TYPE_TRACK_FIRST) {
        eventType = TD_EVENT_TYPE;
        char *firstCheckIDString = (char *)firstCheckID.data();
        if (string(firstCheckIDString).size() == 0) {
            finalDic.SetString("#first_check_id", ta_cpp_helper::getDeviceID().c_str());
        } else {
            finalDic.SetString("#first_check_id", firstCheckIDString);
        }
    } else if (isTrackEvent && (eventType == TD_EVENT_TYPE_TRACK_UPDATE || eventType == TD_EVENT_TYPE_TRACK_OVERWRITE)) {
        finalDic.SetString("#event_id", event_id);
    }

    timeb t;
    ftime(&t);
    finalDic.SetDateTime("#time", t.time, t.millitm);
    

    finalDic.SetString("#uuid", ta_cpp_helper::getEventID());
    
    
    if (account_id_.size()>0) {
        finalDic.SetString("#account_id", account_id_);
    }
    
    
    if (distinct_id_.size()>0) {
        finalDic.SetString("#distinct_id", distinct_id_);
    }
    
    
    if (eventType.size()>0) {
        finalDic.SetString("#type", (char*)eventType.c_str());
    }
    
   
    if (isTrackEvent && event_name.size()>0) {
        finalDic.SetString("#event_name", event_name);
    }
    
    if (isTrackEvent) {
        // 预制属性
        propertyDic.SetString("#lib_version", TD_LIB_VERSION);
		
        #if defined(_WIN32)
        propertyDic.SetString("#os", "Windows");
        #elif defined(__APPLE__)
        propertyDic.SetString("#os", "Mac");
        #endif
        propertyDic.SetString("#device_id", ta_cpp_helper::getDeviceID());
		propertyDic.SetString("#lib", TD_LIB_NAME);
    }
    
    propertyDic.MergeFrom(properties);
    finalDic.SetObject("properties", propertyDic);
    
    std::vector<utils::TDJSONObject> data;
    data.push_back(finalDic);
    flushDic.SetList("data", data);
    flushDic.SetString("#app_id", appid_);
    timeb t1;
    ftime(&t1);
    flushDic.SetDateTime("#flush_time", t1.time, t1.millitm);
    
    consumer_->Send(flushDic);
    return true;
}

void ThinkingAnalyticsAPI::Track(const string &event_name, const PropertiesNode &properties) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE,
                            event_name,
                            properties,
                            ""
                            "");
    }
}


void ThinkingAnalyticsAPI::Track(utils::TDFirstEvent* event) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE_TRACK_FIRST,
                            event->mEventName,
                            event->mProperties,
                            event->mExtraId,
                            "");
    }
}

void ThinkingAnalyticsAPI::Track(utils::TDUpdatableEvent* event) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE_TRACK_UPDATE,
                            event->mEventName,
                            event->mProperties,
                            "",
                            event->mExtraId);
    }
}

void ThinkingAnalyticsAPI::Track(utils::TDOverWritableEvent* event) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE_TRACK_OVERWRITE,
                            event->mEventName,
                            event->mProperties,
                            "",
                            event->mExtraId);
    }
}



void ThinkingAnalyticsAPI::Track(const string &event_name) {
    PropertiesNode properties_node;
    Track(event_name, properties_node);
}


void ThinkingAnalyticsAPI::Login(const string &login_id) {
    if (instance_) {
        instance_->account_id_ = login_id;
		const char *path = instance_->staging_file_path_.c_str();
        ta_cpp_helper::updateAccount(instance_->appid_.c_str(), login_id.c_str(), path);
    }
}

void ThinkingAnalyticsAPI::LogOut() {
    if (instance_) {
        string login_id = "";
        instance_->account_id_ = login_id;
		const char *path = instance_->staging_file_path_.c_str();
        ta_cpp_helper::updateAccount(instance_->appid_.c_str(), login_id.c_str(), path);
    }
}

void ThinkingAnalyticsAPI::Identify(const string &distinct_id) {
    if (instance_) {
        instance_->distinct_id_ = distinct_id;
		const char *path = instance_->staging_file_path_.c_str();
        ta_cpp_helper::updateDistinctId(instance_->appid_.c_str(), distinct_id.c_str(), path);
    }
}

string ThinkingAnalyticsAPI::DistinctID() {
    return instance_ ? instance_->distinct_id_ :  "";
}


void ThinkingAnalyticsAPI::UserSet(const utils::TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_SET, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserSetOnce(const utils::TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_SETONCE, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserAdd(const utils::TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_APPEND, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserAppend(const utils::TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_ADD, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserDelete() {
    if (instance_) {
        utils::TDJSONObject  node;
        instance_->AddUser(TD_EVENT_TYPE_USER_DEL, node);
    }
    
}
void ThinkingAnalyticsAPI::UserUnset(string propertyName)  {
    if (instance_) {
        utils::TDJSONObject  obj;
        obj.SetNumber(propertyName, 0);
        instance_->AddUser(TD_EVENT_TYPE_USER_UNSET, obj);
    }
}


bool ThinkingAnalyticsAPI::IsEnableLog() {
    return instance_ ? instance_->consumer_->enable_log_ : false;
}

string ThinkingAnalyticsAPI::StagingFilePath() {
    return instance_ ? instance_->staging_file_path_ :  "";
}

ThinkingAnalyticsAPI::ThinkingAnalyticsAPI(const string &server_url,
                                           const string &appid,
                                           const string &data_file_path)
: consumer_(new DefaultConsumer(server_url,
                                appid,
                                data_file_path)),
staging_file_path_(data_file_path),
distinct_id_(""){}

ThinkingAnalyticsAPI::ThinkingAnalyticsAPI(const string &server_url,
                                           const string &appid)
: consumer_(new DefaultConsumer(server_url,
                                appid,
                                "")) {}

ThinkingAnalyticsAPI::~ThinkingAnalyticsAPI() {
    if (consumer_ != NULL) {
        delete consumer_;
        consumer_ = NULL;
    }
}

}
