#include <iostream>
#include <ta_analytics_sdk.h>
#include "ta_json_object.h"
#include <thread>
#if defined(_WIN32)
#include <windows.h>
#endif
using namespace thinkingdata;
using namespace std;
std::thread threads[5];
void* myThread(int n)
{
    cout << "线程1" << endl;
    TDJSONObject json;
    json.SetString("#country", "韩国");
    json.SetString("#country_code", "JP");
    json.SetNumber("micro_decimal", -0.2482);
    json.SetString("device_id", "hlEYEb31BXPWhZNbmXDf69aDh7A8l4cGwaku9vQnjtpKIJnJ9S");
    json.SetNumber("level", 71);
    vector<TDJSONObject> list1;
    TDJSONObject json1;
    json1.SetString("skin_name", "远游之枪|激情绿茵|逐梦之星");
    json1.SetNumber("new_type", 0);
    json1.SetNumber("ename", 132);
    vector<string> str1;
    str1.push_back("远游之枪");
    str1.push_back("激情绿茵");
    str1.push_back("逐梦之星");
    json1.SetList("skins_name", str1);
    json1.SetString("cname", "马可波罗");
    json1.SetBool("is_on", true);
    json1.SetString("begin_time", "2021-05-08 23:55:54");
    json1.SetNumber("hero_type", 5);
    json1.SetString("hero_new", "new");
    json1.SetString("title", "远游之枪");
    list1.push_back(json1);

    TDJSONObject json2;
    json2.SetString("skin_name", "半神之弓|精灵王|阿尔法小队|辉光之辰|黄金射手座");
    json2.SetNumber("new_type", 0);
    json2.SetNumber("ename", 169);
    vector<string> str2;
    str2.push_back("半神之弓");
    str2.push_back("精灵王");
    str2.push_back("阿尔法小队");
    str2.push_back("辉光之辰");
    str2.push_back("黄金射手座");
    json2.SetList("skins_name", str2);
    json2.SetString("cname", "后羿");
    json2.SetBool("is_on", true);
    json2.SetString("begin_time", "2021-08-30 18:34:48");
    json2.SetNumber("hero_type", 5);
    json2.SetString("title", "半神之弓");
    json2.SetString("new_name", "月亮");
    list1.push_back(json2);
    list1.push_back(json2);
    list1.push_back(json2);

    json.SetList("object_list", list1);
    json.SetNumber("#zone_offset", -5);
    json.SetString("draw_type", "7连抽");
    json.SetString("#data_source", "Import_Tools");
    json.SetNumber("diamond_amount", 119);
    vector<string> str3;
    str3.push_back("2003");
    str3.push_back("1003");
    str3.push_back("2001");
    str3.push_back("1003");
    str3.push_back("1001");
    str3.push_back("1001");
    str3.push_back("1007");
    json.SetList("card_list", str3);
    TDJSONObject json3;
    json3.SetString("skin_name", "齐天大圣|地狱火|西部大镖客|美猴王|至尊宝|全息碎影|大圣娶亲");
    json3.SetNumber("new_type", 0);
    json3.SetNumber("ename", 167);
    json3.SetNumber("hero_type2", 1);
    json3.SetString("cname", "孙悟空");
    json3.SetString("begin_time", "2021-09-16 14:54:50");
    json3.SetBool("is_on", true);
    json3.SetNumber("hero_type", 5);
    json3.SetString("title", "齐天大圣");
    json3.SetString("new_name", "太阳");
    json.SetObject("object", json3);
    json.SetNumber("n", n);
    for (int i = 0; i < 100; i++) {
        json.SetNumber("i", i);
        ThinkingAnalyticsAPI::Track("test_event_109", json);

    }
    ThinkingAnalyticsAPI::Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100000));
    return 0;
}
void callback(string reason){
    TDJSONObject json;
    json.SetString("encrypt_error_reason",reason);
    ThinkingAnalyticsAPI::Track("ta_encrypt_error",json);
}
bool flag = true;
void display(int code, const string& str)
{
    cout << code << str << endl;
//    if(code == 1004){
//        thread t = thread(callback,str);
//        t.detach();
//    }
}
TDJSONObject GetDynamicSuperProperties(){
    TDJSONObject json;
    json.SetString("dy_name","jack");
    json.SetString("dy_age","18");
    json.SetNumber("dy_number",100);
    return json;
}

int main(){
    cout << "Hello, World!" << endl;
    ThinkingAnalyticsAPI::EnableLogType(LOGCONSOLE);
    const string server_url = "https://receiver-ta-preview.thinkingdata.cn";
    const string appid = "40eddce753cd4bef9883a01e168c3df0";
    TDConfig config;
    config.appid = appid;
    config.server_url = server_url;
    config.enableAutoCalibrated = true;
    config.mode = TDMode::TD_NORMAL;
    config.databaseLimit = 3000;
    config.dataExpression = 15;
    config.EnableEncrypt(1,"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAti6FnWGv7Lggzg/R8hQa4GEtd2ucfntqo6Xkf1sPwCIfndr2u6KGPhWQ24bFUKgtNLDuKnUAg1C/OEEL8uONJBdbX9XpckO67tRPSPrY3ufNIxsCJ9td557XxUsnebkOZ+oC1Duk8/ENx1pRvU6S4c+UYd6PH8wxw1agD61oJ0ju3CW0aZNZ2xKcWBcIU9KgYTeUtawrmGU5flod88CqZc8VKB1+nY0tav023jvxwkM3zgQ6vBWIU9/aViGECB98YEzJfZjcOTD6zvqsZc/WRnUNhBHFPGEwc8ueMvzZNI+FP0pUFLVRwVoYbj/tffKbxGExaRFIcgP73BIW6/6nQwIDAQAB");
    ThinkingAnalyticsAPI::Init(config);
    ThinkingAnalyticsAPI::SetDynamicSuperProperties(GetDynamicSuperProperties);
    std::int64_t timeStamp = 1686567601647;
//    ThinkingAnalyticsAPI::CalibrateTime(timeStamp);
//    ThinkingAnalyticsAPI::Init(server_url, appid);
    ThinkingAnalyticsAPI::registerTECallback(display);
    TDJSONObject json;
    json.SetString("name", "jack");
    json.SetString("#country", "韩国");
    json.SetString("#country_code", "JP");
    json.SetNumber("micro_decimal", -0.2482);
    json.SetString("device_id", "hlEYEb31BXPWhZNbmXDf69aDh7A8l4cGwaku9vQnjtpKIJnJ9S");
    json.SetNumber("level", 71);
    vector<TDJSONObject> list1;
    TDJSONObject json1;
    json1.SetString("skin_name", "远游之枪|激情绿茵|逐梦之星");
    json1.SetNumber("new_type", 0);
    json1.SetNumber("ename", 132);
    vector<string> str1;
    str1.push_back("远游之枪");
    str1.push_back("激情绿茵");
    str1.push_back("逐梦之星");
    json1.SetList("skins_name", str1);
    json1.SetString("cname", "马可波罗");
    json1.SetBool("is_on", true);
    json1.SetString("begin_time", "2021-05-08 23:55:54");
    json1.SetNumber("hero_type", 5);
    json1.SetString("hero_new", "new");
    json1.SetString("title", "远游之枪");
    list1.push_back(json1);

    TDJSONObject json2;
    json2.SetString("skin_name", "半神之弓|精灵王|阿尔法小队|辉光之辰|黄金射手座");
    json2.SetNumber("new_type", 0);
    json2.SetNumber("ename", 169);
    vector<string> str2;
    str2.push_back("半神之弓");
    str2.push_back("精灵王");
    str2.push_back("阿尔法小队");
    str2.push_back("辉光之辰");
    str2.push_back("黄金射手座");
    json2.SetList("skins_name", str2);
    json2.SetString("cname", "后羿");
    json2.SetBool("is_on", true);
    json2.SetString("begin_time", "2021-08-30 18:34:48");
    json2.SetNumber("hero_type", 5);
    json2.SetString("title", "半神之弓");
    json2.SetString("new_name", "月亮");
    list1.push_back(json2);
    list1.push_back(json2);
    list1.push_back(json2);

    json.SetList("object1_list", list1);
    json.SetNumber("zone_offset", -5);
    json.SetString("draw_type", "7连抽");
    json.SetString("#data_source", "Import_Tools");
    json.SetNumber("diamond_amount", 119);
    vector<string> str3;
    str3.push_back("2003");
    str3.push_back("1003");
    str3.push_back("2001");
    str3.push_back("1003");
    str3.push_back("1001");
    str3.push_back("1001");
    str3.push_back("1007");
    json.SetList("card_list", str3);
    TDJSONObject json3;
    json3.SetString("skin_name", "齐天大圣|地狱火|西部大镖客|美猴王|至尊宝|全息碎影|大圣娶亲");
    json3.SetNumber("new_type", 0);
    json3.SetNumber("ename", 167);
    json3.SetNumber("hero_type2", 1);
    json3.SetString("cname", "孙悟空");
    json3.SetString("begin_time", "2021-09-16 14:54:50");
    json3.SetBool("is_on", true);
    json3.SetNumber("hero_type", 5);
    json3.SetString("title", "齐天大圣");
    json3.SetString("new_name", "太阳");
    json.SetObject("object", json3);
//    for (int i = 0; i < 5; i++) {
//        threads[i] = std::thread(myThread, i);
//    }
//    for (auto& t : threads) {
//         t.join();
//    }
//    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    ThinkingAnalyticsAPI::Track("test_event_2",json);
    ThinkingAnalyticsAPI::Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(100000));
    return 0;
}