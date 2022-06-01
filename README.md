# cpp-sdk


一、初始化 SDK
1.1 初始化SDK

```c++
#include "ta_analytics_sdk.h"


// 暂存文件路径, 暂只支持windows
// const string staging_file_path = "./";
   
// 服务端数据接收地址
const string server_url = "http://receiver.ta.thinkingdata.cn";
const string appid = "1b1c1fef65e3482bad5c9d0e6a823356";
   
// mac 初始化
thinkingdata::ThinkingAnalyticsAPI::Init(server_url, appid);
// windows 初始化
thinkingdata::ThinkingAnalyticsAPI::Init(server_url, appid, staging_file_path);

```

二、设置用户 ID
在使用C++ SDK 之后，SDK 默认会使用随机 UUID 作为每个用户的访客 ID，该 ID 将会作为用户在未登录状态下身份识别 ID。需要注意的是，默认访客 ID 在用户重新安装游戏以及更换设备时将会变更。
2.1 设置访客 ID（可选）
如果您的游戏对每个用户有自己的访客 ID 管理体系，则您可以调用 identify 来设置访客 ID:
```c++
thinkingdata::ThinkingAnalyticsAPI::Login("Login12345");

如果需要获得访客 ID，可以调用 getDistinctId 获取：
// 获取distictid
printf("distictid :%s", thinkingdata::ThinkingAnalyticsAPI::DistinctID().c_str());
```
2.2 设置与清除账号 ID
在用户进行登录时，可调用 login 来设置用户的账号 ID，在设置完账号 ID 后，将会以账号 ID 作为用户标识 ID，并且设置的账号 ID 将会在调用 logout 之前一直保留：
// 设置账号 ID
```C++
thinkingdata::ThinkingAnalyticsAPI::Login("Login12345");

// 清除账号 ID
thinkingdata::ThinkingAnalyticsAPI::LogOut();
```

注意：该方法不会上传用户登录、用户登出等事件。
三、上传事件
通过 thinkingdata::ThinkingAnalyticsAPI::Track 上报事件及其属性。
3.1 上传事件
建议您根据先前梳理的文档来设置事件的属性以及发送信息的条件。事件名称是 string 类型，只能以字母开头，可包含数字，字母和下划线 "_"，长度最大为 50 个字符，对字母大小写不敏感。
```c++
// 记录一个行为事件
thinkingdata::PropertiesNode event_properties;
event_properties.SetString("name1", "name1");
event_properties.SetNumber("test_number_int", 3);
event_properties.SetNumber("test_number_double", 3.14);
event_properties.SetBool("test_bool", true);
std::string test_string = "测试字符串1";
event_properties.SetString("test_stl_string1", test_string);
event_properties.SetDateTime("test_time1", time(NULL), 0);
std::vector<std::string> test_list;
test_list.push_back("item11");
test_list.push_back("item21");
event_properties.SetList("test_list1", test_list);
thinkingdata::ThinkingAnalyticsAPI::Track("CPP_event", event_properties);
```
- 事件属性是 PropertiesNode 类型，其中每个元素代表一个属性；
- 事件属性 Key 为属性名称，为 string 类型，规定只能以字母开头，包含数字，字母和下划线 "_"，长度最大为 50 个字符，对字母大小写不敏感；
- 属性值支持六种类型：字符串、数值类、bool、List 类型。
四、用户属性
TA 平台目前支持的用户属性设置接口为 user_set、user_setOnce、user_add、user_unset、user_delete、user_append.
4.1 user_set
对于一般的用户属性，您可以调用 user_set 来进行设置，使用该接口上传的属性将会覆盖原有的属性值，如果之前不存在该用户属性，则会新建该用户属性。
```c++
thinkingdata::utils::TDJSONObject userProperties1;
userProperties1.SetString("userSet_key", "userSet_value");
thinkingdata::ThinkingAnalyticsAPI::UserSet(userProperties1);
```
属性格式要求与事件属性保持一致。
4.2 user_setOnce
如果您要上传的用户属性只要设置一次，则可以调用 user_setOnce 来进行设置，当该属性之前已经有值的时候，将会忽略这条信息：
```c++
thinkingdata::utils::TDJSONObject userProperties2;
userProperties2.SetString("userSetOnce_key", "userSetOnce_value");
userProperties2.SetNumber("userSetOnce_int",1);
thinkingdata::ThinkingAnalyticsAPI::UserSetOnce(userProperties2);
```
属性格式要求与事件属性保持一致。
4.3 user_add
当您要上传数值型的属性时，您可以调用 UserAdd 来对该属性进行累加操作，如果该属性还未被设置，则会赋值 0 后再进行计算，可传入负值，等同于相减操作。
```c++
thinkingdata::utils::TDJSONObject userProperties4;
userProperties4.SetNumber("userAdd_int",1);
thinkingdata::ThinkingAnalyticsAPI::UserAdd(userProperties4);
```

设置的属性key为字符串，Value 只允许为数值。
4.4 user_unset
如果您需要重置用户的某个属性，可以调用 UserUnset 将该用户指定用户属性的值清空，此接口支持传入字符串或者列表类型的参数:
```c++
thinkingdata::ThinkingAnalyticsAPI::UserUnset("userUnset_key");
```
传入值为被清空属性的 Key 值。
4.5 user_delete
如果您要删除某个用户，可以调用 UserDelete 将这名用户删除，您将无法再查询该名用户的用户属性，但该用户产生的事件仍然可以被查询到。
```c++
thinkingdata::ThinkingAnalyticsAPI::UserDelete();
```
4.6 user_append
您可以调用 UserAppend 为 List 类型的用户属性追加元素:
```c++
thinkingdata::utils::TDJSONObject userProperties3;
vector<string> listValue1;
listValue1.push_back("XX");
userProperties3.SetList("userAppend_key",listValue1);
thinkingdata::ThinkingAnalyticsAPI::UserAppend(userProperties1);
```
五、进阶功能
C++ SDK 支持上报三种特殊类型事件: 首次事件、可更新事件、可重写事件。
5.1 首次事件
首次事件是指针对某个设备或者其他维度的 ID，只会记录一次的事件。例如在一些场景下，您可能希望记录在某个设备上第一次发生的事件，则可以用首次事件来上报数据。
```c++
thinkingdata::utils::TDJSONObject jsonObject1;
jsonObject1.SetString("test","test");
thinkingdata::utils::TDFirstEvent *firstEvent = new thinkingdata::utils::TDFirstEvent("firstEvent",jsonObject1);
// 默认情况下使用设备维度
// firstEvent->setFirstCheckId("firstCheckID");
thinkingdata::ThinkingAnalyticsAPI::Track(firstEvent);
```
注意：由于在服务端完成对是否首次的校验，首次事件默认会延时 1 小时入库。
5.2 可更新事件
您可以通过可更新事件实现特定场景下需要修改事件数据的需求。可更新事件需要指定标识该事件的 ID，并在创建可更新事件对象时传入。TA 后台将根据事件名和事件 ID 来确定需要更新的数据。
```c++
thinkingdata::utils::TDJSONObject jsonObject2;
jsonObject2.SetString("test","test");
jsonObject2.SetNumber("status", 3);
jsonObject2.SetNumber("price", 5);
thinkingdata::utils::TDUpdatableEvent *updatableEvent = new thinkingdata::utils::TDUpdatableEvent("updateEvent",jsonObject2,"12345");
thinkingdata::ThinkingAnalyticsAPI::Track(updatableEvent);
```
5.3 可重写事件
可重写事件与可更新事件类似，区别在于可重写事件会用最新的数据完全覆盖历史数据，从效果上看相当于删除前一条数据，并入库最新的数据。TA 后台将根据事件名和事件 ID 来确定需要更新的数据。
```c++
thinkingdata::utils::TDJSONObject jsonObject3;
jsonObject3.SetString("test","test");
thinkingdata::utils::TDOverWritableEvent *overWritableEvent = new thinkingdata::utils::TDOverWritableEvent("overWriteEvent",jsonObject3,"12345");
thinkingdata::ThinkingAnalyticsAPI::Track(overWritableEvent);
```
六、其他配置选项
6.2 打印上传数据 Log
```c++
thinkingdata::ThinkingAnalyticsAPI::EnableLog(true);
```

