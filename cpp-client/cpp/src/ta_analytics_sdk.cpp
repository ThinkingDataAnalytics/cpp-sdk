#include "ta_analytics_sdk.h"
#include "ta_json_object.h"
#include "ta_cpp_send.h"
#include "ta_cJSON.h"
#include <functional>
#include <iostream>
#include <utility>
#include <sys/timeb.h>
#include "ta_cpp_helper.h"
#include "ta_event_task.h"
#include "ta_sqlite.h"
#include "ta_cpp_utils.h"
#include "ta_calibrated_time.h"
#include "ta_flush_task.h"
#include <thread>
#if defined(_WIN32)
#include <windows.h>
#include <iostream>
#include <fstream>
#else
#include <cmath>
#include <pthread.h>
#include <sys/time.h>
#endif

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
const static string TD_EVENT_TYPE_USER_UNIQ_APPEND     = "user_uniq_append";


namespace thinkingdata {

    mutex ta_superProperty_mtx;
    mutex ta_distinct_mtx;
    mutex ta_account_mtx;
    mutex ta_timer_mtx;


void taCJsonToTDJson(tacJSON *myjson, TDJSONObject &property);

void taCJsonArrayToTDJsonArray(tacJSON *myjson, TDJSONObject &property) {
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
            taCJsonToTDJson(obj, _obj);
            objs.push_back(_obj);
            obj = obj->next;
        }
        property.SetList(myjson->string, objs);
    }
}

void taCJsonToTDJson(tacJSON *myjson, TDJSONObject &property) {
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
            taCJsonToTDJson(obj, _obj);
            property.SetObject(obj->string, _obj);
        } else if (obj->type == tacJSON_Array) {
            taCJsonArrayToTDJsonArray(obj, property);
        }
        obj = obj->next;
    }
}

ThinkingAnalyticsEvent::ThinkingAnalyticsEvent(const string &eventName, const TDJSONObject& properties) {
    this->mEventName = eventName;
    this->mProperties = properties;
}
TDFirstEvent::TDFirstEvent(const string & eventName, const TDJSONObject & properties):ThinkingAnalyticsEvent(eventName,properties) {
    this->mType = FIRST;
    this->mExtraId = "";
}

void TDFirstEvent::setFirstCheckId(const string & firstCheckId) {
    this->mExtraId = firstCheckId;
}
TDUpdatableEvent::TDUpdatableEvent(const string & eventName, const TDJSONObject & properties, const string & eventId):ThinkingAnalyticsEvent(eventName,properties) {
    this->mExtraId = eventId;
    this->mType = UPDATABLE;
}
TDOverWritableEvent::TDOverWritableEvent(const string & eventName, const TDJSONObject & properties, const string & eventId):ThinkingAnalyticsEvent(eventName,properties) {
    this->mExtraId = eventId;
    this->mType = OVERWRITABLE;
}



void TDJSONObject::ValueNode::JsonNodeToString(const TDJSONObject::ValueNode &node,
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
TDConfig::~TDConfig() {

}

void TDConfig::EnableEncrypt(int version, const string &publicKey) {
    if(version >0 && !publicKey.empty()){
        this->version = version;
        this->publicKey = publicKey;
        this->enableEncrypt = true;
    }
}

ThinkingAnalyticsAPI *ThinkingAnalyticsAPI::instance_ = NULL;

void ThinkingAnalyticsAPI::fetchRemoteConfigCallback(bool calibrateTime,bool encrypt) {
    if(instance_ && instance_->httpSend_){
        Response res = instance_->httpSend_->fetchRemoteConfig();
        if(res.code_ == 200){
            ta_cpp_helper::printSDKLog("[ThinkingData] Info: Get remote config success, "+res.body_);
            try{
                tacJSON* root_obj = NULL;
                TDJSONObject config;
                root_obj = tacJSON_Parse(res.body_.c_str());
                if (root_obj != NULL && root_obj->type == tacJSON_Object) {
                    stringToTDJson(root_obj, config);
                    TDJSONObject data = config.properties_map_["data"].object_data_;
                    if(calibrateTime){
                        //Automatically start time calibration
                        int64_t timeStamp = static_cast<int64_t>(data.properties_map_["server_timestamp"].value_.number_value);
                        ThinkingAnalyticsAPI::CalibrateTime(timeStamp);
                    }
                    if(encrypt){
                        TDJSONObject secret_key = data.properties_map_["secret_key"].object_data_;
                        string  key = secret_key.properties_map_["key"].string_data_;
                        double v = secret_key.properties_map_["version"].value_.number_value;
                        int version = static_cast<int>(floor(v));
                        instance_->m_sqlite->updateSecretKey(version,key);
                    }
                    vector<string> disableList = data.properties_map_["disable_event_list"].list_data_;
                    instance_->disableEventList = disableList;
                    ta_cpp_helper::flush_bulk_size =static_cast<int>(data.properties_map_["sync_batch_size"].value_.number_value);
                    ta_cpp_helper::flush_interval =static_cast<int>(data.properties_map_["sync_interval"].value_.number_value);
                }
                tacJSON_Delete(root_obj);
            } catch (runtime_error &e) {

            }
        }
    }
}

bool ThinkingAnalyticsAPI::Init(TDConfig &config) {
    if (!instance_) {
        string data_file_path = "";
        string server_url = config.server_url;
        string appid = config.appid;
        if (server_url.empty()) {
            ta_cpp_helper::printSDKLog("[ThinkingData] Error: serverUrl can not be empty");
            return false;
        }

        if (appid.empty()) {
            ta_cpp_helper::printSDKLog("[ThinkingData] Error: appId can not be empty");
            return false;
        }

        if(config.databaseLimit>5000){
            ta_cpp_helper::mini_database_limit = config.databaseLimit;
        }
        if(config.dataExpression > 10){
            ta_cpp_helper::data_expression = config.dataExpression*24*60*60;
        }

        TATaskQueue* dataTaskQue = new (std::nothrow) TATaskQueue();
        if (dataTaskQue == nullptr) {
            ta_cpp_helper::printSDKLog("[ThinkingData] Error:  Failed to allocate memory for dataTaskQue");
            return false;
        }


        ThinkingAnalyticsAPI* ins = new (std::nothrow) ThinkingAnalyticsAPI(server_url, appid);
        if (ins == nullptr) {
            ta_cpp_helper::printSDKLog("[ThinkingData] Error: Failed to allocate memory for ThinkingAnalyticsAPI Init");
            return false;
        }

        bool initStatus;
        TASqliteDataQueue* sqlite = new (std::nothrow) TASqliteDataQueue(appid,initStatus,config.enableEncrypt,config.version,config.publicKey);
        if (sqlite == nullptr || !initStatus) {
            ta_cpp_helper::printSDKLog("[ThinkingData] Error: Failed to allocate memory for TASqliteDataQueue Init");
            return false;
        }

        TAHttpSend* httpSend = new (std::nothrow) TAHttpSend(server_url, appid);
        if (httpSend == nullptr) {
            ta_cpp_helper::printSDKLog("[ThinkingData] Error: Failed to allocate memory for TAHttpSend Init");
            return false;
        }

        TDSystemInfo* tdSystemInfo = new (std::nothrow) TDSystemInfo();
        if(tdSystemInfo == nullptr){
            ta_cpp_helper::printSDKLog("[ThinkingData] Error: Failed to allocate memory for TDTimeCalibrated Init");
            return false;
        }

        // init dataTaskQue networkTaskQue
        TATaskQueue::m_ta_dataTaskQue = dataTaskQue;
        TATaskQueue::m_ta_dataTaskQue->Start();

        // init TA instance
        instance_ = ins;
        instance_->appid_ = appid;
        instance_->server_url_ = server_url;
        instance_->httpSend_ = httpSend;
        instance_->m_sqlite = sqlite;
        instance_->tdSystemInfo = tdSystemInfo;
        instance_->mode = config.mode;

        if(config.mode == TDMode::TD_NORMAL) {
            TATaskQueue* networkTaskQue = new (std::nothrow) TATaskQueue();
            if (dataTaskQue == nullptr) {
                ta_cpp_helper::printSDKLog("[ThinkingEngine] Failed to allocate memory for networkTaskQue");
                return false;
            }
            TATaskQueue::m_ta_networkTaskQue = networkTaskQue;
            TATaskQueue::m_ta_networkTaskQue->Start();

            TDFlushTask* ftask = new (std::nothrow) TDFlushTask();
            if(ftask == nullptr){
                ta_cpp_helper::printSDKLog("[ThinkingData] Error: Failed to allocate memory for TDFlushTask Init");
                return false;
            }
            instance_->flushTask = ftask;
            instance_->flushTask->Start();
        }

        // get local data
        string accountId = ta_cpp_helper::loadAccount(appid.c_str(), data_file_path.c_str());
        string distinctId = ta_cpp_helper::loadDistinctId(appid.c_str(), data_file_path.c_str());
        string deviceId = ta_cpp_helper::getDeviceID();
        string oldSuperPropertyString = ta_cpp_helper::loadSuperProperty(appid.c_str(), data_file_path.c_str());

        if (accountId.length() != 0)  {
            instance_->account_id_ = accountId;
        }

        if (distinctId.size() != 0)  {
            instance_->distinct_id_ = distinctId;
        } else {
            instance_->distinct_id_ = deviceId;
        }

        if (deviceId.size() != 0) {
            instance_->device_id_ = deviceId;
        }

        tacJSON* root_obj = NULL;
        if(oldSuperPropertyString.empty() != true) {
            root_obj = tacJSON_Parse(oldSuperPropertyString.c_str());
            if (root_obj != NULL && root_obj->type == tacJSON_Object) {
                taCJsonToTDJson(root_obj, instance_->m_superProperties);
            }
        }
        tacJSON_Delete(root_obj);

        //Pull configuration information from the server
        thread t = thread(fetchRemoteConfigCallback,config.enableAutoCalibrated,config.enableEncrypt);
        t.detach();

        string result = "[ThinkingData] Info: ThinkingData SDK initialize success, AppId: = " + appid + ", ServerUrl = " + server_url + ", DeviceId = " + ta_cpp_helper::getDeviceID()+ ", LibVersion  = " + TD_LIB_VERSION;
        ta_cpp_helper::printSDKLog(result);
    }

    return true;
}

bool ThinkingAnalyticsAPI::Init(const string &server_url,
                                const string &appid) {

    if (!instance_) {
        TDConfig tdConfig;
        tdConfig.appid = appid;
        tdConfig.server_url = server_url;
        return Init(tdConfig);
    }
    return false;
}

void ThinkingAnalyticsAPI::CalibrateTime(int64_t &timestamp) {
    if(instance_){
        instance_->tdSystemInfo->enableTimeCalibrated(timestamp);
    }
}

void ThinkingAnalyticsAPI::EnableLog(bool enable) {
    if(enable){
        TAEnableLog::setTALogType(LOGCONSOLE);
    }
}

void ThinkingAnalyticsAPI::EnableLogType(TALogType type) {
    TAEnableLog::setTALogType(type);
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

    if(containsKey(instance_->disableEventList,event_name)){
        return false;
    }

    TDJSONObject flushDic;
    TDJSONObject finalDic;
    TDJSONObject propertyDic;

    string eventType = action_type;
    string event_id = eventID;

    bool isTrackEvent = (eventType == TD_EVENT_TYPE || eventType == TD_EVENT_TYPE_TRACK_FIRST || eventType == TD_EVENT_TYPE_TRACK_UPDATE || eventType == TD_EVENT_TYPE_TRACK_OVERWRITE);

    if (isTrackEvent) {
        ta_superProperty_mtx.lock();
        propertyDic.MergeFrom(m_superProperties);
        ta_superProperty_mtx.unlock();
        if(instance_->dynamicSuperProperties != nullptr){
            propertyDic.MergeFrom(instance_->dynamicSuperProperties());
        }
    }

    if (eventType == TD_EVENT_TYPE_TRACK_FIRST) {
        eventType = TD_EVENT_TYPE;
        if (firstCheckID.size() > 0) {
            finalDic.SetString("#first_check_id", firstCheckID);
        } else {
            finalDic.SetString("#first_check_id", ta_cpp_helper::getDeviceID().c_str());
        }
    } else if (eventType == TD_EVENT_TYPE_TRACK_UPDATE || eventType == TD_EVENT_TYPE_TRACK_OVERWRITE) {
        if (event_id.size() > 0) {
            finalDic.SetString("#event_id", event_id);
        }
    }

    timeb t;
    try{
        instance_->tdSystemInfo->getTime(&t);
    }catch(const std::exception&){
        ftime(&t);
    }
    finalDic.SetDateTime("#time", t.time, t.millitm);

    // finalDic.SetString("#uuid", ta_cpp_helper::getEventID());

    ta_account_mtx.lock();
    if (account_id_.size()>0) {
        finalDic.SetString("#account_id", account_id_);
    }
    ta_account_mtx.unlock();

    ta_distinct_mtx.lock();
    if (distinct_id_.size()>0) {
        finalDic.SetString("#distinct_id", distinct_id_);
    }
    ta_distinct_mtx.unlock();

    if (eventType.size()>0) {
        finalDic.SetString("#type", eventType);
    }

    if (isTrackEvent && event_name.size()>0) {
        finalDic.SetString("#event_name", event_name);
    }

    if (isTrackEvent) {
        ta_timer_mtx.lock();
        if(instance_->trackTimer.count(event_name)>0){
            int64_t t1 = instance_->trackTimer[event_name];
            int64_t t2 = getSystemElapsedRealTime();
            double duration = static_cast<double>(t2-t1)/1000.0;
            if(duration > 24*60*60){
                duration = 24*60*60;
            }
            if(duration<0){
                duration = 0;
            }
            propertyDic.SetNumber("#duration", duration);
            instance_->trackTimer.erase(event_name);
        }
        ta_timer_mtx.unlock();

        propertyDic.MergeFrom(instance_->tdSystemInfo->presetProperties);
    }
    
    propertyDic.MergeFrom(properties);
    finalDic.SetObject("properties", propertyDic);
   
    const string json_record = TDJSONObject::ToJson(finalDic);

    if(instance_->mode == TDMode::TD_NORMAL){
        TASqiteInsetTask *sqiteInsetTask = new (std::nothrow) TASqiteInsetTask(httpSend_,m_sqlite, json_record, appid_);
        if (sqiteInsetTask == nullptr) {
            ta_cpp_helper::printSDKLog("[ThinkingEngine] Failed to allocate memory for TASqiteInsetTask Init");
            return false;
        }
        shared_ptr<TAITask> task(sqiteInsetTask);
        TATaskQueue::m_ta_dataTaskQue->PushTask(task);
    }else{
        TADebugTask *debugTask = new (std::nothrow) TADebugTask(httpSend_,appid_,device_id_,json_record,instance_->mode == TDMode::TD_DEBUG_ONLY);
        if (debugTask == nullptr) {
            ta_cpp_helper::printSDKLog("[ThinkingEngine] Failed to allocate memory for TADebugTask Init");
            return false;
        }
        shared_ptr<TAITask> task(debugTask);
        TATaskQueue::m_ta_dataTaskQue->PushTask(task);
    }
    return true;
}

void ThinkingAnalyticsAPI::InnerFlush() {
    TAFlushTask *_flushTask = new TAFlushTask(m_sqlite, httpSend_, appid_);
    if (_flushTask == nullptr) {
        ta_cpp_helper::printSDKLog("[ThinkingEngine] Failed to allocate memory for TAFlushTask Init");
        return ;
    }
    shared_ptr<TAITask> flushTask(_flushTask);
    TATaskQueue::m_ta_dataTaskQue->PushTask(flushTask);
}

void ThinkingAnalyticsAPI::Flush() {
    if (instance_) {
        if(instance_->mode == TDMode::TD_NORMAL){
            instance_->InnerFlush();
        }
    }
}

void ThinkingAnalyticsAPI::registerTECallback(void (*p)(int, const string&)) {
    if (instance_) {
        instance_->funcs.push_back(p);
    }
}

void ThinkingAnalyticsAPI::SetDynamicSuperProperties(DynamicSuperProperties dynamicSuperProperties) {
    if(instance_){
        instance_->dynamicSuperProperties = dynamicSuperProperties;
    }
}


vector<void(*)(int,const string&)> ThinkingAnalyticsAPI::getTECallback(){
    if (instance_) {
        return instance_->funcs;
    }
    return {};
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
        ta_account_mtx.lock();
        instance_->account_id_ = login_id;
		const char *path = instance_->staging_file_path_.c_str();
        ta_cpp_helper::updateAccount(instance_->appid_.c_str(), login_id.c_str(), path);
        ta_account_mtx.unlock();
        ta_cpp_helper::printSDKLog("[ThinkingData] Info: Login SDK, AccountId = "+login_id);
    }
}

void ThinkingAnalyticsAPI::LogOut() {
    if (instance_) {
        ta_account_mtx.lock();
        string login_id = "";
        instance_->account_id_ = login_id;
		const char *path = instance_->staging_file_path_.c_str();
        ta_cpp_helper::updateAccount(instance_->appid_.c_str(), login_id.c_str(), path);
        ta_account_mtx.unlock();
        ta_cpp_helper::printSDKLog("[ThinkingData] Info: Logout SDK");
    }
}

void ThinkingAnalyticsAPI::Identify(const string &distinct_id) {
    if (instance_) {
        ta_distinct_mtx.lock();
        if (distinct_id.size()>0) {
            instance_->distinct_id_ = distinct_id;
        }
        else {
            instance_->distinct_id_ = instance_->device_id_;
        }
		const char *path = instance_->staging_file_path_.c_str();
        ta_cpp_helper::updateDistinctId(instance_->appid_.c_str(), distinct_id.c_str(), path);
        ta_distinct_mtx.unlock();
        ta_cpp_helper::printSDKLog("[ThinkingData] Info: Setting distinct ID, DistinctId = "+distinct_id);
    }
}

void ThinkingAnalyticsAPI::SetSuperProperty(const TDJSONObject &properties){
    if (instance_) {

        ta_superProperty_mtx.lock();
        instance_->m_superProperties.MergeFrom(properties);
       
        string superProperty = TDJSONObject::ToJson(instance_->m_superProperties);
#if defined(_WIN32) && defined(_MSC_VER)
        if (!CheckUtf8Valid(superProperty.c_str())) {
            char* str = G2U(superProperty.c_str());;
            superProperty = string(str);
            delete str;
            ta_cpp_helper::updateSuperProperty(instance_->appid_.c_str(), superProperty.c_str());

        } else {
            ta_cpp_helper::updateSuperProperty(instance_->appid_.c_str(), superProperty.c_str());
        }
#else
        ta_cpp_helper::updateSuperProperty(instance_->appid_.c_str(), superProperty.c_str());
#endif  
        ta_superProperty_mtx.unlock();
    }
}

void ThinkingAnalyticsAPI::GetSuperProperties(TDJSONObject &properties) {
    if(instance_){
        ta_superProperty_mtx.lock();
        properties = instance_->m_superProperties;
        ta_superProperty_mtx.unlock();
    }
}

void ThinkingAnalyticsAPI::GetPresetProperties(TDJSONObject &properties) {
    if (instance_ && instance_->tdSystemInfo) {
        properties = instance_->tdSystemInfo->presetProperties;
    }
}

void ThinkingAnalyticsAPI::ClearSuperProperty(){
    if (instance_) {
        ta_superProperty_mtx.lock();
        instance_->m_superProperties.Clear();
        ta_cpp_helper::updateSuperProperty(instance_->appid_.c_str(), TDJSONObject::ToJson(instance_->m_superProperties).c_str());
        ta_superProperty_mtx.unlock();
    }
}

void ThinkingAnalyticsAPI::UnsetSuperProperties(const string &propertyName) {
    if (instance_) {
        ta_superProperty_mtx.lock();
        instance_->m_superProperties.Remove(propertyName);
        string superProperty = TDJSONObject::ToJson(instance_->m_superProperties);
        ta_cpp_helper::updateSuperProperty(instance_->appid_.c_str(), superProperty.c_str());
        ta_superProperty_mtx.unlock();
    }
}


string ThinkingAnalyticsAPI::DistinctID() {
    return instance_ ? instance_->distinct_id_ :  "";
}

string ThinkingAnalyticsAPI::GetDeviceId() {
    return instance_ ? instance_->device_id_ :  "";
}

void ThinkingAnalyticsAPI::TimeEvent(const string &event_name) {
    if(instance_){
        ta_timer_mtx.lock();
        instance_->trackTimer[event_name] = getSystemElapsedRealTime();
        ta_timer_mtx.unlock();
    }
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

void ThinkingAnalyticsAPI::UserUniqAppend(const TDJSONObject &properties) {
    if (instance_) {
        instance_->AddUser(TD_EVENT_TYPE_USER_UNIQ_APPEND, properties);
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

string ThinkingAnalyticsAPI::StagingFilePath() {
    return instance_ ? instance_->staging_file_path_ :  "";
}

ThinkingAnalyticsAPI::ThinkingAnalyticsAPI(const string& server_url, const string& appid): server_url_(server_url), appid_(appid) {}

ThinkingAnalyticsAPI::~ThinkingAnalyticsAPI() {}

void ThinkingAnalyticsAPI::UnInit()
{
    if (instance_ != nullptr)
    {

        if(instance_->flushTask != nullptr){
            delete instance_->flushTask;
            instance_->flushTask = nullptr;
        }

        if (TATaskQueue::m_ta_dataTaskQue != nullptr) {
            delete TATaskQueue::m_ta_dataTaskQue;
        }
        if (TATaskQueue::m_ta_networkTaskQue != nullptr) {
            delete TATaskQueue::m_ta_networkTaskQue;
        }

        if (instance_->httpSend_ != nullptr) {
            delete instance_->httpSend_;
            instance_->httpSend_ = nullptr;
        }

        if (instance_->m_sqlite != nullptr) {
            instance_->m_sqlite->isStop = true;
            instance_->m_sqlite->unInit();
            delete instance_->m_sqlite;
            instance_->m_sqlite = nullptr;
        }

        if(instance_->tdSystemInfo != nullptr){
            delete instance_->tdSystemInfo;
            instance_->tdSystemInfo = nullptr;
        }

        delete instance_;
        instance_ = nullptr;
    }
}

}
