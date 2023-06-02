#include <iostream>
#include <ta_analytics_sdk.h>
#include "ta_json_object.h"
#include <thread>

#if defined(_WIN32)
#include <synchapi.h>
#endif

#include <pthread.h>
#include <unistd.h>


using namespace thinkingdata;
using namespace std;

void * myThread(void * arg)
{
    cout << "线程1" << endl;
    TDJSONObject json;
    json.SetString("#country","韩国");
    json.SetString("#country_code","JP");
    json.SetNumber("micro_decimal",-0.2482);
    json.SetString("device_id","hlEYEb31BXPWhZNbmXDf69aDh7A8l4cGwaku9vQnjtpKIJnJ9S");
    json.SetNumber("level",71);
    vector<TDJSONObject> list1;
    TDJSONObject json1;
    json1.SetString("skin_name","远游之枪|激情绿茵|逐梦之星");
    json1.SetNumber("new_type",0);
    json1.SetNumber("ename",132);
    vector<string> str1;
    str1.push_back("远游之枪");
    str1.push_back("激情绿茵");
    str1.push_back("逐梦之星");
    json1.SetList("skins_name",str1);
    json1.SetString("cname","马可波罗");
    json1.SetBool("is_on",true);
    json1.SetString("begin_time","2021-05-08 23:55:54");
    json1.SetNumber("hero_type",5);
    json1.SetString("hero_new","new");
    json1.SetString("title","远游之枪");
    list1.push_back(json1);

    TDJSONObject json2;
    json2.SetString("skin_name","半神之弓|精灵王|阿尔法小队|辉光之辰|黄金射手座");
    json2.SetNumber("new_type",0);
    json2.SetNumber("ename",169);
    vector<string> str2;
    str2.push_back("半神之弓");
    str2.push_back("精灵王");
    str2.push_back("阿尔法小队");
    str2.push_back("辉光之辰");
    str2.push_back("黄金射手座");
    json2.SetList("skins_name",str2);
    json2.SetString("cname","后羿");
    json2.SetBool("is_on",true);
    json2.SetString("begin_time","2021-08-30 18:34:48");
    json2.SetNumber("hero_type",5);
    json2.SetString("title","半神之弓");
    json2.SetString("new_name","月亮");
    list1.push_back(json2);
    list1.push_back(json2);
    list1.push_back(json2);

    json.SetList("object_list",list1);
    json.SetNumber("#zone_offset",-5);
    json.SetString("draw_type","7连抽");
    json.SetString("#data_source","Import_Tools");
    json.SetNumber("diamond_amount",119);
    vector<string> str3;
    str3.push_back("2003");
    str3.push_back("1003");
    str3.push_back("2001");
    str3.push_back("1003");
    str3.push_back("1001");
    str3.push_back("1001");
    str3.push_back("1007");
    json.SetList("card_list",str3);
    TDJSONObject json3;
    json3.SetString("skin_name","齐天大圣|地狱火|西部大镖客|美猴王|至尊宝|全息碎影|大圣娶亲");
    json3.SetNumber("new_type",0);
    json3.SetNumber("ename",167);
    json3.SetNumber("hero_type2",1);
    json3.SetString("cname","孙悟空");
    json3.SetString("begin_time","2021-09-16 14:54:50");
    json3.SetBool("is_on",true);
    json3.SetNumber("hero_type",5);
    json3.SetString("title","齐天大圣");
    json3.SetString("new_name","太阳");
    json.SetObject("object",json3);

    for (int i = 0;i<100;i++) {
        ThinkingAnalyticsAPI::Track("test_event_2",json);
    }
    ThinkingAnalyticsAPI::Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(90000));
    return 0;
}

void * myThread1(void * arg)
{
    cout << "线程2" << endl;
    TDJSONObject json;
    json.SetString("#country","韩国");
    json.SetString("#country_code","JP");
    json.SetNumber("micro_decimal",-0.2482);
    json.SetString("device_id","hlEYEb31BXPWhZNbmXDf69aDh7A8l4cGwaku9vQnjtpKIJnJ9S");
    json.SetNumber("level",71);
    vector<TDJSONObject> list1;
    TDJSONObject json1;
    json1.SetString("skin_name","远游之枪|激情绿茵|逐梦之星");
    json1.SetNumber("new_type",0);
    json1.SetNumber("ename",132);
    vector<string> str1;
    str1.push_back("远游之枪");
    str1.push_back("激情绿茵");
    str1.push_back("逐梦之星");
    json1.SetList("skins_name",str1);
    json1.SetString("cname","马可波罗");
    json1.SetBool("is_on",true);
    json1.SetString("begin_time","2021-05-08 23:55:54");
    json1.SetNumber("hero_type",5);
    json1.SetString("hero_new","new");
    json1.SetString("title","远游之枪");
    list1.push_back(json1);

    TDJSONObject json2;
    json2.SetString("skin_name","半神之弓|精灵王|阿尔法小队|辉光之辰|黄金射手座");
    json2.SetNumber("new_type",0);
    json2.SetNumber("ename",169);
    vector<string> str2;
    str2.push_back("半神之弓");
    str2.push_back("精灵王");
    str2.push_back("阿尔法小队");
    str2.push_back("辉光之辰");
    str2.push_back("黄金射手座");
    json2.SetList("skins_name",str2);
    json2.SetString("cname","后羿");
    json2.SetBool("is_on",true);
    json2.SetString("begin_time","2021-08-30 18:34:48");
    json2.SetNumber("hero_type",5);
    json2.SetString("title","半神之弓");
    json2.SetString("new_name","月亮");
    list1.push_back(json2);
    list1.push_back(json2);
    list1.push_back(json2);

    json.SetList("object_list",list1);
    json.SetNumber("#zone_offset",-5);
    json.SetString("draw_type","7连抽");
    json.SetString("#data_source","Import_Tools");
    json.SetNumber("diamond_amount",119);
    vector<string> str3;
    str3.push_back("2003");
    str3.push_back("1003");
    str3.push_back("2001");
    str3.push_back("1003");
    str3.push_back("1001");
    str3.push_back("1001");
    str3.push_back("1007");
    json.SetList("card_list",str3);
    TDJSONObject json3;
    json3.SetString("skin_name","齐天大圣|地狱火|西部大镖客|美猴王|至尊宝|全息碎影|大圣娶亲");
    json3.SetNumber("new_type",0);
    json3.SetNumber("ename",167);
    json3.SetNumber("hero_type2",1);
    json3.SetString("cname","孙悟空");
    json3.SetString("begin_time","2021-09-16 14:54:50");
    json3.SetBool("is_on",true);
    json3.SetNumber("hero_type",5);
    json3.SetString("title","齐天大圣");
    json3.SetString("new_name","太阳");
    json.SetObject("object",json3);


    for (int i = 0;i<100;i++) {
        ThinkingAnalyticsAPI::Track("test_event_2",json);
    }
    ThinkingAnalyticsAPI::Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(90000));

    return 0;
}
void display(int a,const string& b)
{
    cout << "a=" << a << endl;
    cout << "b=" << b << endl;
}

int main(){
    const string server_url = "https://receiver-ta-preview.thinkingdata.cn";
    const string appid = "40eddce753cd4bef9883a01e168c3df0";
    ThinkingAnalyticsAPI::EnableLogType(thinkingdata::LOGTXT);
    ThinkingAnalyticsAPI::Init(server_url, appid);
//    ThinkingAnalyticsAPI::EnableLog(true);
    ThinkingAnalyticsAPI::registerTECallback(display);
    TDJSONObject json;
    json.SetString("#country","韩国");
    json.SetString("#country_code","JP");
    json.SetNumber("micro_decimal",-0.2482);
    json.SetString("device_id","hlEYEb31BXPWhZNbmXDf69aDh7A8l4cGwaku9vQnjtpKIJnJ9S");
    json.SetNumber("level",71);
    vector<TDJSONObject> list1;
    TDJSONObject json1;
    json1.SetString("skin_name","远游之枪|激情绿茵|逐梦之星");
    json1.SetNumber("new_type",0);
    json1.SetNumber("ename",132);
    vector<string> str1;
    str1.push_back("远游之枪");
    str1.push_back("激情绿茵");
    str1.push_back("逐梦之星");
    json1.SetList("skins_name",str1);
    json1.SetString("cname","马可波罗");
    json1.SetBool("is_on",true);
    json1.SetString("begin_time","2021-05-08 23:55:54");
    json1.SetNumber("hero_type",5);
    json1.SetString("hero_new","new");
    json1.SetString("title","远游之枪");
    list1.push_back(json1);

    TDJSONObject json2;
    json2.SetString("skin_name","半神之弓|精灵王|阿尔法小队|辉光之辰|黄金射手座");
    json2.SetNumber("new_type",0);
    json2.SetNumber("ename",169);
    vector<string> str2;
    str2.push_back("半神之弓");
    str2.push_back("精灵王");
    str2.push_back("阿尔法小队");
    str2.push_back("辉光之辰");
    str2.push_back("黄金射手座");
    json2.SetList("skins_name",str2);
    json2.SetString("cname","后羿");
    json2.SetBool("is_on",true);
    json2.SetString("begin_time","2021-08-30 18:34:48");
    json2.SetNumber("hero_type",5);
    json2.SetString("title","半神之弓");
    json2.SetString("new_name","月亮");
    list1.push_back(json2);
    list1.push_back(json2);
    list1.push_back(json2);

    json.SetList("object_list",list1);
    json.SetNumber("#zone_offset",-5);
    json.SetString("draw_type","7连抽");
    json.SetString("#data_source","Import_Tools");
    json.SetNumber("diamond_amount",119);
    vector<string> str3;
    str3.push_back("2003");
    str3.push_back("1003");
    str3.push_back("2001");
    str3.push_back("1003");
    str3.push_back("1001");
    str3.push_back("1001");
    str3.push_back("1007");
    json.SetList("card_list",str3);
    TDJSONObject json3;
    json3.SetString("skin_name","齐天大圣|地狱火|西部大镖客|美猴王|至尊宝|全息碎影|大圣娶亲");
    json3.SetNumber("new_type",0);
    json3.SetNumber("ename",167);
    json3.SetNumber("hero_type2",1);
    json3.SetString("cname","孙悟空");
    json3.SetString("begin_time","2021-09-16 14:54:50");
    json3.SetBool("is_on",true);
    json3.SetNumber("hero_type",5);
    json3.SetString("title","齐天大圣");
    json3.SetString("new_name","太阳");
    json.SetObject("object",json3);

//    pthread_t pthread;
//    pthread_create(&pthread, NULL,myThread , NULL);
//    pthread_join(pthread, NULL);
//
//    pthread_t pthread1;
//    pthread_create(&pthread1, NULL,myThread1 , NULL);
//    pthread_join(pthread1, NULL);


    for (int i = 0;i<1;i++) {
        ThinkingAnalyticsAPI::Track("test_event_2",json);
    }

    ThinkingAnalyticsAPI::Flush();



#if defined(_WIN32)
    cout << "windows"<<endl;
    #elif defined(__APPLE__)
        cout << "apple"<<endl;
    #endif

    std::this_thread::sleep_for(std::chrono::milliseconds(100000));

    return 0;
}



//int main() {
//    cout << "Hello, World!" << endl;
//
//    const string server_url = "http://receiver.ta.thinkingdata.cn";
//    const string appid = "1b1c1fef65e3482bad5c9d0e6a823356";
//    TDJSONObject event_properties;
//    TDJSONObject jsonObject1;
//    vector<string> super_list;
//    TDJSONObject item;
//    vector<TDJSONObject> super_array;
//    TDJSONObject item1;
//    TDJSONObject jsonObject2;
//    TDJSONObject jsonObject3;
//    // user_set
//    TDJSONObject userProperties1;
//    // user_setOnce
//    TDJSONObject userProperties2;
//    // user_append
//    TDJSONObject userProperties3;
//    vector<string> listValue1;
//    // user_add
//    TDJSONObject userProperties4;
//    TDJSONObject ppp;
//
//    ThinkingAnalyticsAPI::Init(server_url, appid);
//    ThinkingAnalyticsAPI::EnableLog(true);
//
//    ThinkingAnalyticsAPI::Login("Login123456");
//    ThinkingAnalyticsAPI::Identify("Identify123456");
//
//    TDJSONObject super_properties;
//    super_properties.SetString("super_key", "super_value");
//
//
//    super_list.push_back("item");
//    super_list.push_back("index");
//    super_properties.SetList("super_list", super_list);
//
//
//    item.SetString("key", "value");
//    super_properties.SetObject("super_item", item);
//
//
//    item1.SetString("key", "value");
//    super_array.push_back(item1);
//    super_properties.SetList("super_arr", super_array);
//
////    ThinkingAnalyticsAPI::SetSuperProperty(super_properties);
//    /*
//    ThinkingAnalyticsAPI::LogOut();
//    ThinkingAnalyticsAPI::Identify("");
//    */
//
////    ThinkingAnalyticsAPI::ClearSuperProperty();
//
//
//    printf("get api distictid:%s", ThinkingAnalyticsAPI::DistinctID().c_str());
//
//
//    event_properties.SetString("name1", "name1");
//    event_properties.SetNumber("test_number_int", 3);
//    event_properties.SetNumber("test_number_double", 3.14);
//    event_properties.SetBool("test_bool", true);
//    string test_string = "test_string";
//    event_properties.SetString("test_stl_string1", test_string);
//    event_properties.SetDateTime("test_time1", time(NULL), 0);
//    vector<string> test_list;
//    test_list.push_back("item11");
//    test_list.push_back("item21");
//    event_properties.SetList("test_list1", test_list);
//    ThinkingAnalyticsAPI::Track("CPP_event", event_properties);
//
//
//    jsonObject1.SetString("test", "test");
//    TDFirstEvent *firstEvent = new TDFirstEvent("firstEvent", jsonObject1);
//    firstEvent->setFirstCheckId("firstCheckID");
//    ThinkingAnalyticsAPI::Track(firstEvent);
//
//
//    jsonObject2.SetString("test", "test");
//    jsonObject2.SetNumber("status", 3);
//    jsonObject2.SetNumber("price", 5);
//    TDUpdatableEvent *updatableEvent = new TDUpdatableEvent("updateEvent", jsonObject2, "12345");
//    ThinkingAnalyticsAPI::Track(updatableEvent);
//
//    jsonObject3.SetString("test", "test");
//    TDOverWritableEvent *overWritableEvent = new TDOverWritableEvent("overWriteEvent", jsonObject3, "12345");
//    ThinkingAnalyticsAPI::Track(overWritableEvent);
//
//    userProperties1.SetString("userSet_key", "userSet_value");
//    ThinkingAnalyticsAPI::UserSet(userProperties1);
//
//    userProperties2.SetString("userSetOnce_key", "userSetOnce_value");
//    userProperties2.SetNumber("userSetOnce_int", 1);
//    ThinkingAnalyticsAPI::UserSetOnce(userProperties2);
//
//    // user_del
//    ThinkingAnalyticsAPI::UserDelete();
//
//    listValue1.push_back("XX");
//    userProperties3.SetList("userAppend_key", listValue1);
//    ThinkingAnalyticsAPI::UserAppend(userProperties1);
//
//    userProperties4.SetNumber("userAdd_int", 1);
//    ThinkingAnalyticsAPI::UserAdd(userProperties4);
//
//    // user_unset
//    ThinkingAnalyticsAPI::UserUnset("userUnset_key");
//
//    cout << "\nmain_thread_task1" << endl;
//
//    for (int i = 0; i < 1000; ++i) {
//#if defined(_WIN32)
//        Sleep(1 * 1000);
//#else
//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//#endif
//
//
//
//        ppp.SetString("key", "value_"+ to_string(i));
//        ThinkingAnalyticsAPI::Track("test", ppp);
////        ThinkingAnalyticsAPI::Flush();
//    }
//
//#if defined(_WIN32)
//    Sleep(4 * 1000);
//#else
//    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
//#endif
//    cout << "\nmain_thread_end" << endl;
//
//    ThinkingAnalyticsAPI::UserUnset("userUnset_key");
//    return 0;
//}
