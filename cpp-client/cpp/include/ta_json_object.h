//
// Created by wwango on 2022/11/14.
//
#if defined(_WIN32)
#define THINKINGDATA_API __declspec(dllexport)
#endif
#ifndef UNTITLED1_TA_JSON_OBJECT_H
#define UNTITLED1_TA_JSON_OBJECT_H

#include <cstring>
#include <ctime>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace thinkingdata {
    
    #if defined(_WIN32)
    class THINKINGDATA_API TDJSONObject;
    #endif

    using namespace std;

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

        void SetDateTime(const string &property_name, const string &value);

        void Clear();

        void SetObject(const string &property_name, const TDJSONObject &value);

        static string ToJson(const TDJSONObject &node);

        class ValueNode;

        std::map<string, ValueNode> properties_map_;

        static void DumpNode(const TDJSONObject &node, string *buffer);

        static void UnInit(TDJSONObject* node);

        void MergeFrom(const TDJSONObject &another_node);

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

        static void JsonNodeToString(const ValueNode &node, string *buffer);


        ValueNodeType node_type_;
        std::vector<TDJSONObject> list_obj_;
        TDJSONObject object_data_;
        string string_data_;

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

    private:
        static void DumpString(const string &value, string *buffer);

        static void DumpList(const std::vector<string> &value, string *buffer);

        static void DumpList(const std::vector<TDJSONObject> &value, string *buffer);

        static void DumpDateTime(const time_t &seconds, int milliseconds,
                                 string *buffer);

        static void DumpNumber(double value, string *buffer);

        static void DumpNumber(int64_t value, string *buffer);
        
        std::vector<string> list_data_;
       
    };
};

#endif //UNTITLED1_TA_JSON_OBJECT_H
