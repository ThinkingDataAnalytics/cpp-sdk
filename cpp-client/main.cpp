#include <iostream>
#include <ta_analytics_sdk.h>
#include "ta_json_object.h"
#include <thread>

#if defined(_WIN32)
#include <synchapi.h>
#endif


using namespace thinkingdata;
using namespace std;

int main() {
    cout << "Hello, World!" << endl;

    const string server_url = "http://receiver.ta.thinkingdata.cn";
    const string appid = "1b1c1fef65e3482bad5c9d0e6a823356";
    TDJSONObject event_properties;
    TDJSONObject jsonObject1;
    vector<string> super_list;
    TDJSONObject item;
    vector<TDJSONObject> super_array;
    TDJSONObject item1;
    TDJSONObject jsonObject2;
    TDJSONObject jsonObject3;
    // user_set
    TDJSONObject userProperties1;
    // user_setOnce
    TDJSONObject userProperties2;
    // user_append
    TDJSONObject userProperties3;
    vector<string> listValue1;
    // user_add
    TDJSONObject userProperties4;
    TDJSONObject ppp;

    ThinkingAnalyticsAPI::Init(server_url, appid);
    ThinkingAnalyticsAPI::EnableLog(true);

    ThinkingAnalyticsAPI::Login("Login123456");
    ThinkingAnalyticsAPI::Identify("Identify123456");

    TDJSONObject super_properties;
    super_properties.SetString("super_key", "super_value");


    super_list.push_back("item");
    super_list.push_back("index");
    super_properties.SetList("super_list", super_list);


    item.SetString("key", "value");
    super_properties.SetObject("super_item", item);


    item1.SetString("key", "value");
    super_array.push_back(item1);
    super_properties.SetList("super_arr", super_array);

//    ThinkingAnalyticsAPI::SetSuperProperty(super_properties);
    /*
    ThinkingAnalyticsAPI::LogOut();
    ThinkingAnalyticsAPI::Identify("");
    */

//    ThinkingAnalyticsAPI::ClearSuperProperty();


    printf("get api distictid:%s", ThinkingAnalyticsAPI::DistinctID().c_str());


    event_properties.SetString("name1", "name1");
    event_properties.SetNumber("test_number_int", 3);
    event_properties.SetNumber("test_number_double", 3.14);
    event_properties.SetBool("test_bool", true);
    string test_string = "test_string";
    event_properties.SetString("test_stl_string1", test_string);
    event_properties.SetDateTime("test_time1", time(NULL), 0);
    vector<string> test_list;
    test_list.push_back("item11");
    test_list.push_back("item21");
    event_properties.SetList("test_list1", test_list);
    ThinkingAnalyticsAPI::Track("CPP_event", event_properties);


    jsonObject1.SetString("test", "test");
    TDFirstEvent *firstEvent = new TDFirstEvent("firstEvent", jsonObject1);
    firstEvent->setFirstCheckId("firstCheckID");
    ThinkingAnalyticsAPI::Track(firstEvent);


    jsonObject2.SetString("test", "test");
    jsonObject2.SetNumber("status", 3);
    jsonObject2.SetNumber("price", 5);
    TDUpdatableEvent *updatableEvent = new TDUpdatableEvent("updateEvent", jsonObject2, "12345");
    ThinkingAnalyticsAPI::Track(updatableEvent);

    jsonObject3.SetString("test", "test");
    TDOverWritableEvent *overWritableEvent = new TDOverWritableEvent("overWriteEvent", jsonObject3, "12345");
    ThinkingAnalyticsAPI::Track(overWritableEvent);

    userProperties1.SetString("userSet_key", "userSet_value");
    ThinkingAnalyticsAPI::UserSet(userProperties1);

    userProperties2.SetString("userSetOnce_key", "userSetOnce_value");
    userProperties2.SetNumber("userSetOnce_int", 1);
    ThinkingAnalyticsAPI::UserSetOnce(userProperties2);

    // user_del
    ThinkingAnalyticsAPI::UserDelete();

    listValue1.push_back("XX");
    userProperties3.SetList("userAppend_key", listValue1);
    ThinkingAnalyticsAPI::UserAppend(userProperties1);

    userProperties4.SetNumber("userAdd_int", 1);
    ThinkingAnalyticsAPI::UserAdd(userProperties4);

    // user_unset
    ThinkingAnalyticsAPI::UserUnset("userUnset_key");

    cout << "\nmain_thread_task1" << endl;

    for (int i = 0; i < 1000; ++i) {
#if defined(_WIN32)
        Sleep(1 * 1000);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif



        ppp.SetString("key", "value_"+ to_string(i));
        ThinkingAnalyticsAPI::Track("test", ppp);
//        ThinkingAnalyticsAPI::Flush();
    }

    // 主线程睡眠
#if defined(_WIN32)
    Sleep(4 * 1000);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
#endif
    cout << "\nmain_thread_end" << endl;

    ThinkingAnalyticsAPI::UserUnset("userUnset_key");
    return 0;
}
