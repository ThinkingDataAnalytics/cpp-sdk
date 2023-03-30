//
// Created by wwango on 2022/11/14.
//
#include "ta_json_object.h"
#include "ta_cpp_utils.h"
#include <iostream>

#if defined(_WIN32)
#include "windows.h"
#include "stringapiset.h"
#endif


namespace thinkingdata {
    using namespace std;

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

    void TDJSONObject::SetString(const string &property_name, const string &value) {
//    if (value.length() > kStringPropertyValueMaxLength) {
//        std::cerr << "String property '" << property_name
//        << "' is too long, value: " << value << std::endl;
//        return;
//    }

        if (!CheckUtf8Valid(property_name)) {
            std::cerr << "String Key '" << property_name
                << "' is not valid string, key: " << value
                << std::endl;
            return;
        }

        if (!CheckUtf8Valid(value)) {
            std::cerr << "String property '" << property_name
                      << "' is not valid UTF-8 string, value: " << value
                      << std::endl; 
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

    void TDJSONObject::SetDateTime(const string &property_name,
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
            ValueNode::JsonNodeToString(iterator->second, buffer);
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

    void TDJSONObject::MergeFrom(const TDJSONObject &another_node) {
        for (std::map<string, ValueNode>::const_iterator
            iterator = another_node.properties_map_.begin();
            iterator != another_node.properties_map_.end(); ++iterator) {
            properties_map_[iterator->first] = iterator->second;
        }
    }


    void TDJSONObject::UnInit(TDJSONObject* another_node) {
        for (std::map<string, ValueNode>::const_iterator
            iterator = another_node->properties_map_.begin();
            iterator != another_node->properties_map_.end(); ++iterator) {
            if (iterator->second.node_type_ == OBJECT) {
                delete  & iterator->second.object_data_;
            }
        }
    }

    TDJSONObject::ValueNode::ValueNode(double value) : node_type_(NUMBER) {
        value_.number_value = value;
    }

    TDJSONObject::ValueNode::ValueNode(int64_t value) : node_type_(INT) {
        value_.int_value = value;
    }

    TDJSONObject::ValueNode::ValueNode(const string &value)
            : node_type_(STRING),
              string_data_(value) {}

    TDJSONObject::ValueNode::ValueNode(bool value) : node_type_(BOOL) {
        value_.bool_value = value;
    }

    TDJSONObject::ValueNode::ValueNode(const TDJSONObject &value)
            : node_type_(OBJECT) {
        object_data_ = value;
    }

    TDJSONObject::ValueNode::ValueNode(const std::vector<string> &value)
            : node_type_(LIST),
              list_data_(value) {}

    TDJSONObject::ValueNode::ValueNode(const std::vector<TDJSONObject> &value)
            : node_type_(OBJECTS),
              list_obj_(value) {}

    TDJSONObject::ValueNode::ValueNode(time_t seconds, int milliseconds)
            : node_type_(DATETIME) {
        value_.date_time_value.seconds = seconds;
        value_.date_time_value.milliseconds = milliseconds;
    }

};