//
// Created by wwango on 2022/11/10.
//

#include "ta_event_task.h"
#include <sys/timeb.h>
#include "ta_sqlite.h"
#include "ta_json_object.h"
#include "ta_cpp_utils.h"
#include "ta_cpp_helper.h"
#include "ta_cJSON.h"

namespace thinkingdata {

    TATaskQueue* TATaskQueue::m_ta_dataTaskQue = nullptr;
    TATaskQueue* TATaskQueue::m_ta_networkTaskQue = nullptr;

    mutex ta_sqlite_mtx;

    TASqiteInsetTask::TASqiteInsetTask(TAHttpSend* httpSend, TASqliteDataQueue* sqliteQueue, string event, string appid) :m_httpSend(httpSend), m_sqliteQueue(sqliteQueue), m_appid(appid), m_event(event) {}
    TANetworkTask::TANetworkTask(TASqliteDataQueue* sqliteQueue,TAHttpSend* httpSend, string appid): m_sqliteQueue(sqliteQueue), m_httpSend(httpSend), m_appid(appid) {}
    TAFlushTask::TAFlushTask(TASqliteDataQueue *sqliteQueue,TAHttpSend *httpSend, string appid): m_sqliteQueue(sqliteQueue), m_httpSend(httpSend), m_appid(appid) {}

    TASqiteInsetTask::~TASqiteInsetTask() {}
    TANetworkTask::~TANetworkTask() {}
    TAFlushTask::~TAFlushTask() {}

    void TASqiteInsetTask::DoTask() {
        ta_cpp_helper::printSDKLog("[ThinkingEngine] data queue:");
        ta_cpp_helper::printSDKLog(m_event);


        if (isStop == true) {
            return;
        }

        if (m_sqliteQueue == nullptr) {
            return;
        }

        if (m_sqliteQueue->isStop) {
            return;
        }

        ta_sqlite_mtx.lock();
        long messageCount = m_sqliteQueue->addObject(m_event, m_appid);
        ta_sqlite_mtx.unlock();

        if (messageCount >= 30) {
            TANetworkTask *task = new TANetworkTask(m_sqliteQueue, m_httpSend, m_appid);
            if (task == nullptr) {
                ta_cpp_helper::printSDKLog("[ThinkingEngine] Failed to allocate memory for TANetworkTask Init");
                return ;
            }
            shared_ptr<TAITask> networkTask(task);
            TATaskQueue::m_ta_networkTaskQue->PushTask(networkTask);
        }
    }

    void TANetworkTask::DoTask() {

        if (isStop == true) {
            return;
        }

        if (m_sqliteQueue == nullptr) {
            return;
        }

        if (m_sqliteQueue->isStop) {
            return;
        }

        TDJSONObject flushDic;
        vector<tuple<string, string>> records;
        ta_sqlite_mtx.lock();
        m_sqliteQueue->getFirstRecords(50, m_appid,records);
        ta_sqlite_mtx.unlock();

        while (!records.empty()) {

            vector<TDJSONObject> data;
            vector<string> uuids;
            for (auto record : records) {
                uuids.push_back(get<0>(record));
                string s = get<1>(record);
                tacJSON* root_obj = NULL;
                if(!s.empty()) {
                    TDJSONObject dataJson;
                    root_obj = tacJSON_Parse(s.c_str());
                    if (root_obj->type == tacJSON_Object) {
                        stringToTDJson(root_obj, dataJson);
                    }
                    data.push_back(dataJson);
                }
                tacJSON_Delete(root_obj);
            }
            flushDic.SetList("data", data);
            flushDic.SetString("#app_id", m_appid);
            timeb t1;
            ftime(&t1);
            flushDic.SetDateTime("#flush_time", t1.time, t1.millitm);

            string flushStr = TDJSONObject::ToJson(flushDic);

            bool result = m_httpSend->Send(flushDic);
            if (result == true) {
                ta_sqlite_mtx.lock();
                m_sqliteQueue->removeData(uuids);
                m_sqliteQueue->getFirstRecords(50, m_appid,records);
                ta_sqlite_mtx.unlock();
            } else {
                break;
            }
        }
    }

    void TAFlushTask::DoTask() {
        ta_cpp_helper::printSDKLog("[ThinkingEngine] Flush Task");

        if (isStop == true) {
            return;
        }

        if (m_sqliteQueue == nullptr) {
            return;
        }

        if (m_sqliteQueue->isStop) {
            return;
        }

        TANetworkTask *task = new TANetworkTask(m_sqliteQueue, m_httpSend, m_appid);
        if (task == nullptr) {
            ta_cpp_helper::printSDKLog("[ThinkingEngine] Failed to allocate memory for TANetworkTask Init");
            return ;
        }

        shared_ptr<TAITask> networkTask(task);
        TATaskQueue::m_ta_networkTaskQue->PushTask(networkTask);
    }

    void TASqiteInsetTask::Stop() {}
    void TANetworkTask::Stop() {}
    void TAFlushTask::Stop() {}

    TATaskQueue::TATaskQueue() : m_pThread(nullptr), isStop(false) {}

    TATaskQueue::~TATaskQueue() {
//        isStop = true;
        stopWait();
        if (m_pThread != nullptr) {
            if (m_pThread->joinable())
            {
                m_pThread->join();
            }
            delete m_pThread;
            m_pThread = nullptr;
        }
        while (!m_taskQue.empty()) {
            m_taskQue.front()->isStop = true;
            m_taskQue.pop();
        }
    }

    bool TATaskQueue::Start() {
        m_pThread = new(nothrow) thread(&TATaskQueue::ThreadCallBack, this);
        return (m_pThread != NULL);
    }

    void TATaskQueue::PushTask(const shared_ptr<TAITask> &task) {
        std::unique_lock<std::mutex> lock(m_lock);
        m_taskQue.push(task);
        cv.notify_one();
        lock.unlock();
    }

    void TATaskQueue::stopWait() {
        std::unique_lock<std::mutex> lock(m_lock);
        isStop = true;
        cv.notify_all();
        lock.unlock();
    }

    void TATaskQueue::ThreadCallBack() {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        while (!isStop) {
            std::unique_lock<std::mutex> lock(m_lock);
            if (m_taskQue.empty() && !isStop) {
                cv.wait(lock);
            }
            if (!m_taskQue.empty()) {
                shared_ptr<TAITask> tmp = m_taskQue.front();
                if (!isStop && tmp != nullptr) {
                    tmp->DoTask();
                }
                m_taskQue.pop();
                tmp.reset();
            }
            lock.unlock();
        }
    }
};