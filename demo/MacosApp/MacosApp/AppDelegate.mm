//
//  AppDelegate.m
//  MacosApp
//
//  Created by wwango on 2022/10/25.
//

#import "AppDelegate.h"
#import "ta_analytics_sdk.h"

using namespace thinkingdata;

@interface AppDelegate ()


@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    // 服务端数据接收地址
    const string server_url = "http://receiver.ta.thinkingdata.cn";
    const string appid = "1b1c1fef65e3482bad5c9d0e6a823356";
    
       
    // 初始化
    ThinkingAnalyticsAPI::Init(server_url, appid);
    
    ThinkingAnalyticsAPI::Login("Login12345");
    
    // 记录一个行为事件
    TDJSONObject event_properties;
    event_properties.SetString("name1", "name1");//字符串
    event_properties.SetNumber("test_number_int", 3);//数字
    event_properties.SetBool("test_bool", true);//bool
    event_properties.SetDateTime("test_time1", time(NULL), 0);//时间
    std::vector<std::string> test_list;
    test_list.push_back("item11");
    test_list.push_back("item21");
    event_properties.SetList("test_list1", test_list);//数组
    ThinkingAnalyticsAPI::Track("CPP_event", event_properties);
    
    // UserSet
    TDJSONObject userProperties;
    userProperties.SetString("user_name", "TA");
    ThinkingAnalyticsAPI::UserSet(userProperties);
    
    
    // user_set
//    TDJSONObject userProperties1;
//    userProperties1.SetString("userSet_key", "userSet_value");
//    thinkingdata::ThinkingAnalyticsAPI::UserSet(userProperties1);

}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}


@end
