#include "ta_analytics_sdk.h"
#include "ta_cpp_network.h"
#include "ta_json_object.h"
#include "ta_cpp_send.h"
#include "ta_cJSON.h"

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
#include "ta_timer.h"
#include "ta_event_task.h"
#include "ta_sqlite.h"
#include "ta_cJSON.h"

const static string TD_EVENT_TYPE                 = "track";
const static string TD_EVENT_TYPE_TRACK_FIRST     = "track_first";
const static string TD_EVENT_TYPE_TRACK_UPDATE    = "track_update";
const static string TD_EVENT_TYPE_TRACK_OVERWRITE = "track_overwrite";

const static string TD_EVENT_TYPE_USER_DEL        = "user_del";
const static string TD_EVENT_TYPE_USER_ADD        = "user_add";
const static string TD_EVENT_TYPE_USER_SET        = "user_set";
const static string TD_EVENT_TYPE_USER_SETONCE    = "user_setOnce";
const static string TD_EVENT_TYPE_USER_UNSET      = "user_unset";
const static string TD_EVENT_TYPE_USER_APPEND     = "user_append";


namespace thinkingdata {

void myToTDJSON(tacJSON *myjson, TDJSONObject &ppp);

void myArrToTDJSON(tacJSON *myjson, TDJSONObject &ppp) {
    tacJSON *child = myjson->child;
    if (child->type == tacJSON_String) {
        vector<string> objs = vector<string>();
        tacJSON* obj = child;
        while (obj != nullptr) {
            objs.push_back(obj->valuestring);
            obj = obj->next;
        }
        ppp.SetList(myjson->string, objs);
    } else if (child->type == tacJSON_Object) {

        vector<TDJSONObject> objs = vector<TDJSONObject>();
        tacJSON* obj = child;
        while (obj != nullptr) {
            TDJSONObject *ojjjc = new TDJSONObject();
            myToTDJSON(obj, *ojjjc);
            objs.push_back(*ojjjc);
            obj = obj->next;
        }
        ppp.SetList(myjson->string, objs);
    }
}

void myToTDJSON(tacJSON *myjson, TDJSONObject &ppp) {
    tacJSON* obj = myjson->child;
    while (obj != nullptr) {
        if (obj->type == tacJSON_String) {
            ppp.SetString(obj->string,obj->valuestring);
        } else if (obj->type == tacJSON_False) {
            ppp.SetBool(obj->string, false);
        } else if (obj->type == tacJSON_True) {
            ppp.SetBool(obj->string, true);
        } else if (obj->type == tacJSON_Number) {
            ppp.SetNumber(obj->string, obj->valuedouble);
        } else if (obj->type == tacJSON_Object) {
            TDJSONObject *ojjjc = new TDJSONObject();
            myToTDJSON(obj, *ojjjc);
            ppp.SetObject(obj->string, *ojjjc);
        } else if (obj->type == tacJSON_Array) {
            myArrToTDJSON(obj, ppp);
        }
        obj = obj->next;
    }
}

ThinkingAnalyticsEvent::ThinkingAnalyticsEvent(string eventName, TDJSONObject properties) {
    this->mEventName = eventName;
    this->mProperties = properties;
}
TDFirstEvent::TDFirstEvent(string eventName, TDJSONObject properties):ThinkingAnalyticsEvent(eventName,properties) {
    this->mType = FIRST;
    this->mExtraId = "";
}

void TDFirstEvent::setFirstCheckId(string firstCheckId) {
    this->mExtraId = firstCheckId;
}
TDUpdatableEvent::TDUpdatableEvent(string eventName, TDJSONObject properties, string eventId):ThinkingAnalyticsEvent(eventName,properties) {
    this->mExtraId = eventId;
    this->mType = UPDATABLE;
}
TDOverWritableEvent::TDOverWritableEvent(string eventName, TDJSONObject properties, string eventId):ThinkingAnalyticsEvent(eventName,properties) {
    this->mExtraId = eventId;
    this->mType = OVERWRITABLE;
}



void
TDJSONObject::ValueNode::ToStr(const TDJSONObject::ValueNode &node,
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
    ostringstream buf;
    buf.imbue(locale("C"));
    buf << value;
    *buffer += buf.str();
}

void TDJSONObject::ValueNode::DumpNumber(int64_t value, string *buffer) {
    ostringstream buf;
    buf.imbue(locale("C"));
    buf << value;
    *buffer += buf.str();
}

ThinkingAnalyticsAPI *ThinkingAnalyticsAPI::instance_ = NULL;

bool ThinkingAnalyticsAPI::Init(const string &server_url,
                                const string &appid) {
    string oldsuperpstring;

    if (!instance_) {
        string data_file_path = "";
        instance_ = new ThinkingAnalyticsAPI(server_url, appid);
        instance_->httpSend_->Init();
        srand((unsigned)time(NULL));
        
        instance_->distinct_id_ = ta_cpp_helper::getDeviceID();
        instance_->account_id_ = "";
        
        // 恢复distinctid、accountid
        string accountid = ta_cpp_helper::loadAccount(appid.c_str(), data_file_path.c_str());
        string distinctid = ta_cpp_helper::loadDistinctId(appid.c_str(), data_file_path.c_str());

        // 恢复之前的属性
        oldsuperpstring = ta_cpp_helper::loadSuperProperty(appid.c_str(), data_file_path.c_str());
        tacJSON* root_obj = NULL;
        TDJSONObject *superProperties = new TDJSONObject();
        if(oldsuperpstring.empty() != true) {
            root_obj = tacJSON_Parse(oldsuperpstring.c_str());
            if (root_obj->type == tacJSON_Object) {
                myToTDJSON(root_obj, *superProperties);
            }
        }
        instance_->m_superProperties = superProperties;

        if (accountid.length() != 0) {
            instance_->account_id_ = accountid;
        }
        if (distinctid.size() != 0) {
            instance_->distinct_id_ = distinctid;
        }
        
        instance_->appid_ = appid;
        instance_->server_url_ = server_url;

        instance_->m_sqlite = new TASqliteDataQueue(appid);
    }
    return true;
}

void ThinkingAnalyticsAPI::EnableLog(bool enable) {

    if (instance_ != nullptr) {
        instance_->enableLog = enable;
        instance_->httpSend_->EnableLog(enable);
    }

}

void ThinkingAnalyticsAPI::AddUser(string eventType, const TDJSONObject &properties)
{
    AddEvent(eventType, "", properties, "", "");
}


bool ThinkingAnalyticsAPI::AddEvent(const string &action_type,
                                    const string &event_name,
                                    const TDJSONObject &properties,
                                    const string &firstCheckID,
                                    const string &eventID) {

    TDJSONObject flushDic;
    TDJSONObject finalDic;
    TDJSONObject propertyDic;
    
    string first_check_id = firstCheckID;
    string eventType = action_type;
    string event_id = eventID;
    
    bool isTrackEvent = (eventType == TD_EVENT_TYPE || eventType == TD_EVENT_TYPE_TRACK_FIRST || eventType == TD_EVENT_TYPE_TRACK_UPDATE || eventType == TD_EVENT_TYPE_TRACK_OVERWRITE);

    if (isTrackEvent) {
        propertyDic.MergeFrom(*m_superProperties);
    }

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

    const string json_record = TDJSONObject::ToJson(finalDic);
    shared_ptr<TAITask> task(new TASqiteInsetTask(*m_sqlite, json_record, appid_, enableLog));
    ThinkingdataTask::getMDataTaskQue()->PushTask(task);

    long messageCount = m_sqlite->getAllmessageCount(appid_);

    if (enableLog == true) {
        printf("\n[thinkingdata] messageCount: %ld\n", messageCount);
    }

    if(messageCount >= 30) {
        shared_ptr<TAITask> networkTask(new TANetworkTask(*m_sqlite, *httpSend_, appid_, enableLog));
        ThinkingdataTask::getMNetworkTaskQue()->PushTask(networkTask);
    }
    return true;
}

void ThinkingAnalyticsAPI::InnerFlush() {
    shared_ptr<TAITask> networkTask(new TANetworkTask(*m_sqlite, *httpSend_, appid_, enableLog));
    ThinkingdataTask::getMNetworkTaskQue()->PushTask(networkTask);
}

void ThinkingAnalyticsAPI::Flush() {
    if (instance_) {
        instance_->InnerFlush();
    }
}

void ThinkingAnalyticsAPI::Track(const string &event_name, const TDJSONObject &properties) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE,
                            event_name,
                            properties,
                            ""
                            "");
    }
}


void ThinkingAnalyticsAPI::Track(TDFirstEvent* event) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE_TRACK_FIRST,
                            event->mEventName,
                            event->mProperties,
                            event->mExtraId,
                            "");
    }
}

void ThinkingAnalyticsAPI::Track(TDUpdatableEvent* event) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE_TRACK_UPDATE,
                            event->mEventName,
                            event->mProperties,
                            "",
                            event->mExtraId);
    }
}

void ThinkingAnalyticsAPI::Track(TDOverWritableEvent* event) {
    if (instance_) {
        instance_->AddEvent(TD_EVENT_TYPE_TRACK_OVERWRITE,
                            event->mEventName,
                            event->mProperties,
                            "",
                            event->mExtraId);
    }
}



void ThinkingAnalyticsAPI::Track(const string &event_name) {
    TDJSONObject properties_node;
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

void ThinkingAnalyticsAPI::SetSuperProperty(const TDJSONObject &properties){
    if (instance_) {
        instance_->m_superProperties->MergeFrom(properties);
        ta_cpp_helper::updateSuperProperty(instance_->appid_.c_str(), TDJSONObject::ToJson(*instance_->m_superProperties).c_str());
    }
}

void ThinkingAnalyticsAPI::ClearSuperProperty(){
    if (instance_) {
        instance_->m_superProperties = new TDJSONObject();
        ta_cpp_helper::updateSuperProperty(instance_->appid_.c_str(), TDJSONObject::ToJson(*instance_->m_superProperties).c_str());
    }
}


string ThinkingAnalyticsAPI::DistinctID() {
    return instance_ ? instance_->distinct_id_ :  "";
}


void ThinkingAnalyticsAPI::UserSet(const TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_SET, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserSetOnce(const TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_SETONCE, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserAdd(const TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_ADD, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserAppend(const TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_APPEND, properties);
    }
    
}
void ThinkingAnalyticsAPI::UserDelete() {
    if (instance_) {
        TDJSONObject  node;
        instance_->AddUser(TD_EVENT_TYPE_USER_DEL, node);
    }
    
}
void ThinkingAnalyticsAPI::UserUnset(string propertyName)  {
    if (instance_) {
        TDJSONObject  obj;
        obj.SetNumber(propertyName, 0);
        instance_->AddUser(TD_EVENT_TYPE_USER_UNSET, obj);
    }
}


bool ThinkingAnalyticsAPI::IsEnableLog() {
    return instance_ ? instance_->httpSend_->enable_log_ : false;
}

string ThinkingAnalyticsAPI::StagingFilePath() {
    return instance_ ? instance_->staging_file_path_ :  "";
}

ThinkingAnalyticsAPI::ThinkingAnalyticsAPI(const string &server_url,
                                           const string &appid)
: httpSend_(new TAHttpSend(server_url, appid)) {}

ThinkingAnalyticsAPI::~ThinkingAnalyticsAPI() {
    if (httpSend_ != NULL) {
        delete httpSend_;
        httpSend_ = NULL;
    }
}

}
