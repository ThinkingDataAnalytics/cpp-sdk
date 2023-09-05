
#include "ta_cpp_utils.h"
#include <sstream>
#include <iostream>
#include <iomanip>

#if defined(_WIN32) && defined(_MSC_VER)
#include <atlstr.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif

namespace thinkingdata {

    static bool _taEnableLog;
    static TALogType _logType = LOGNONE;
    //mutex ta_enable_log_mutex;
    bool thinkingdata::TAEnableLog::getEnableLog() {
//        bool _ta_enable_log = false;
//        ta_enable_log_mutex.lock();
//        _ta_enable_log = TAEnableLog::taEnableLog;
//        ta_enable_log_mutex.unlock();
        return _taEnableLog;
    }
    void thinkingdata::TAEnableLog::setEnableLog(bool enableLog) {
        _taEnableLog = enableLog;
//        ta_enable_log_mutex.lock();
//        TAEnableLog::taEnableLog == enableLog;
//        ta_enable_log_mutex.unlock();

    }

    TALogType thinkingdata::TAEnableLog::getTALogType() {
        return _logType;
    }

    void thinkingdata::TAEnableLog::setTALogType(TALogType type) {
        _logType = type;
    }

vector<string> Split(const string &str, const string &pattern) {
  vector<string> res;
  string last;
  if (str == "") return res;
  last = str.substr(str.size() - 1, 1);
  string strs = str;
  if (last != pattern) {
    strs = str + pattern;
  }
  size_t pos = strs.find(pattern);

  while (pos != strs.npos) {
    string temp = strs.substr(0, pos);
    res.push_back(temp);
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(pattern);
  }
  return res;
}

map<string, string> ParserQueryItems(const string &query) {
  map<string, string> result;
  if (query.length() < 1) {
    return result;
  }
  vector<string> query_arr = Split(query, "&");
  for (vector<string>::iterator iter = query_arr.begin();
       iter != query_arr.end(); ++iter) {
    vector<string> item_arr = Split(*iter, "=");
    if (item_arr.size() > 1) {
      string first = item_arr[0];
      string second = item_arr[1];
      result.insert(pair<string, string>(first, second));
    }
  }
  return result;
}

#define CHECK_LEN_END(POS, LEN) \
  if (POS >= LEN) {             \
    _url_errorno = 100;         \
    goto __PARSE_END;           \
  }
#define WALK_SP(POS, LEN, BUF) for (; POS < LEN && BUF[POS] == ' '; POS++)
#define WALK_UNTIL(POS, LEN, BUF, DELC) \
  for (; POS < LEN && BUF[POS] != DELC; POS++)
#define WALK_UNTIL2(POS, LEN, BUF, DELI1, DELI2) \
  for (; POS < LEN && BUF[POS] != DELI1 && BUF[POS] != DELI2; POS++)
#define WALK_UNTIL3(POS, LEN, BUF, DELI1, DELI2, DELI3)         \
  for (; POS < LEN && BUF[POS] != DELI1 && BUF[POS] != DELI2 && \
         BUF[POS] != DELI3;                                     \
       POS++)
#define CHECK_REMAIN_END(POS, LEN, REQ_LEN) \
  if (LEN - POS < REQ_LEN) {                \
    _url_errorno = 100;                     \
    goto __PARSE_END;                       \
  }
#define WALK_CHAR(POS, BUF, DELI) \
  if (BUF[POS++] != DELI) goto __PARSE_END
void UrlParser::parse() {
  int _url_errorno = 0;
  const char *str = mRawUrl.c_str();

  int pos, len, scheme_pos, host_pos, port_pos, path_pos, param_pos, tag_pos;
  pos = 0;
  len = (int)mRawUrl.size();
  WALK_SP(pos, len, str);  // remove preceding spaces.
  if (str[pos] == '/') {
    goto __PARSE_HOST;
  }

  // start protocol scheme
  scheme_pos = pos;
  WALK_UNTIL(pos, len, str, ':');
  CHECK_LEN_END(pos, len);
  scheme = mRawUrl.substr(scheme_pos, pos - scheme_pos);
  CHECK_REMAIN_END(pos, len, 3);
  WALK_CHAR(pos, str, ':');
  WALK_CHAR(pos, str, '/');

// start host address
__PARSE_HOST:
  WALK_CHAR(pos, str, '/');
  host_pos = pos;
  WALK_UNTIL3(pos, len, str, ':', '/', '?');
  if (pos < len) {
    hostName = mRawUrl.substr(host_pos, pos - host_pos);
    if (str[pos] == ':') goto __PARSE_PORT;
    if (str[pos] == '/') goto __PARSE_PATH;
    if (str[pos] == '?') goto __PARSE_PARAM;
  } else {
    hostName = mRawUrl.substr(host_pos, pos - host_pos);
  }

__PARSE_PORT:
  WALK_CHAR(pos, str, ':');
  port_pos = pos;
  WALK_UNTIL2(pos, len, str, '/', '?');
  port = mRawUrl.substr(port_pos, pos - port_pos);
  CHECK_LEN_END(pos, len);
  if (str[pos] == '?') goto __PARSE_PARAM;
__PARSE_PATH:
  path_pos = pos;
  WALK_UNTIL(pos, len, str, '?');
  path = mRawUrl.substr(path_pos, pos - path_pos);
  CHECK_LEN_END(pos, len);
__PARSE_PARAM:
  WALK_CHAR(pos, str, '?');
  param_pos = pos;
  WALK_UNTIL(pos, len, str, '#');
  query = mRawUrl.substr(param_pos, pos - param_pos);

  CHECK_LEN_END(pos, len);
  // start parsing fragment
  WALK_CHAR(pos, str, '#');
  tag_pos = pos;
  fragment = mRawUrl.substr(tag_pos, len - tag_pos);
__PARSE_END:
  return;
}

UrlParser *UrlParser::parseUrl(string urlstr) {
  UrlParser *url = new UrlParser;
  url->mRawUrl = urlstr;
  url->parse();
  url->queryItems = ParserQueryItems(url->query);

  return url;
}

string UrlWithoutQuery(UrlParser *parser) {
  string result = parser->scheme + "://" + parser->hostName;
  if (parser->port.length() > 0) {
    result = result + ":" + parser->port;
  }
  if (parser->path.length() > 0) {
    result = result + parser->path;
  }
  return result;
}

string Splice(const vector<string> &array, const string &pattern) {
  string result;
  vector<string> list(array);
  if (array.size() < 1) {
    return "";
  }
  for (vector<string>::const_iterator iter = list.begin();
       iter != list.end() - 1; ++iter) {
    result += *iter;
    result += pattern;
  }
  string end = list.back();
  result += end;
  return result;
}

bool CheckUtf8Valid(const string& str) {
    const unsigned char* bytes = (const unsigned char*)str.data();
    const unsigned char* begin = bytes;
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

void stringArrayToTDJsonArray(tacJSON *myjson, TDJSONObject &property){
    tacJSON *child = myjson->child;
    if (child->type == tacJSON_String) {
        vector<string> objs = vector<string>();
        tacJSON* obj = child;
        while (obj != nullptr) {
            objs.push_back(obj->valuestring);
            obj = obj->next;
        }
        property.SetList(myjson->string, objs);
    } else if (child->type == tacJSON_Object) {

        vector<TDJSONObject> objs = vector<TDJSONObject>();
        tacJSON* obj = child;
        while (obj != nullptr) {
            TDJSONObject _obj;
            stringToTDJson(obj, _obj);
            objs.push_back(_obj);
            obj = obj->next;
        }
        property.SetList(myjson->string, objs);
    }
}
void stringToTDJson(tacJSON *myjson, TDJSONObject &property){
    tacJSON* obj = myjson->child;
    while (obj != nullptr) {
        if (obj->type == tacJSON_String) {
            property.SetString(obj->string,obj->valuestring);
        } else if (obj->type == tacJSON_False) {
            property.SetBool(obj->string, false);
        } else if (obj->type == tacJSON_True) {
            property.SetBool(obj->string, true);
        } else if (obj->type == tacJSON_Number) {
            property.SetNumber(obj->string, obj->valuedouble);
        } else if (obj->type == tacJSON_Object) {
            TDJSONObject _obj;
            stringToTDJson(obj, _obj);
            property.SetObject(obj->string, _obj);
        } else if (obj->type == tacJSON_Array) {
            stringArrayToTDJsonArray(obj, property);
        }
        obj = obj->next;
    }
}

bool containsKey(const vector<string>& list, const string& key){
    if (find(list.begin(), list.end(), key) != list.end()){
        return true;
    }
    return false;
}

int64_t getSystemElapsedRealTime(){
    #if defined(_WIN32)
    DWORD ticks = GetTickCount();
    return static_cast<int64_t>(ticks);
    #elif defined(__APPLE__)
    uint64_t time = mach_absolute_time();
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    uint64_t nanoseconds = time * info.numer / info.denom;
    int64_t t2 = static_cast<int64_t>(nanoseconds / 1000000);
    return (t2 << 1) >> 1;
    #endif
    return 0;
}


#if defined(_WIN32) && defined(_MSC_VER)
char* G2U(const char* gb2312)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    if (wstr) delete[] wstr;
    return str;

}

char* U2G(const char* utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    if (wstr) delete[] wstr;
    return str;
}
#endif


}
