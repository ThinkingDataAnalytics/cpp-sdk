//
// Created by wwango on 2022/11/10.
//

#include "ta_event_task.h"
#include <sys/timeb.h>
#include "ta_sqlite.h"
#include "ta_json_object.h"

namespace thinkingdata {

    mutex ta_sqlite_mtx;

    TASqiteInsetTask::TASqiteInsetTask(TASqliteDataQueue &sqliteQueue, string event, string appid, bool enableLog) : m_sqliteQueue(sqliteQueue),m_appid(appid), m_event(event), m_enableLog(enableLog){}
    TASqiteDeleteTask::TASqiteDeleteTask(TASqliteDataQueue &sqliteQueue, vector<string> uuids, bool enableLog): m_sqliteQueue(sqliteQueue), m_uuids(uuids), m_enableLog(enableLog) {}
    TANetworkTask::TANetworkTask(TASqliteDataQueue &sqliteQueue,TAHttpSend &httpSend, string appid, bool enableLog): m_sqliteQueue(sqliteQueue), m_httpSend(httpSend), m_appid(appid), m_enableLog(enableLog) {}

    TASqiteInsetTask::~TASqiteInsetTask() {
//        printf("\n[thinkingdata] TASqiteInsetTask deinit");
    }
    TASqiteDeleteTask::~TASqiteDeleteTask() {
//        printf("\n[thinkingdata] TASqiteDeleteTask deinit");
    }
    TANetworkTask::~TANetworkTask() {
//        printf("\n[thinkingdata] TANetworkTask deinit");
    }

void TASqiteInsetTask::DoTask() {

        if (m_enableLog == true) {
            printf("\n[thinkingdata] data queue: %s\n", m_event.c_str());
        }

        ta_sqlite_mtx.lock();
        m_sqliteQueue.addObject(m_event, m_appid);
        ta_sqlite_mtx.unlock();
    }

    void TASqiteDeleteTask::DoTask() {
        ta_sqlite_mtx.lock();
        m_sqliteQueue.removeData(m_uuids);
        ta_sqlite_mtx.unlock();
    }

    void TANetworkTask::DoTask() {
        TDJSONObject flushDic;
        ta_sqlite_mtx.lock();
        vector<tuple<string, string>> records = m_sqliteQueue.getFirstRecords(50, m_appid);
        ta_sqlite_mtx.unlock();
        if (records.empty()) return;

        vector<string> data;
        vector<string> uuids;
        string strData = "[";
        for(auto record : records) {
            uuids.push_back(get<0>(record));
            strData += get<1>(record);
            strData += ",";
        }
        strData =  strData.substr(0, strData.size()-1);
        strData += "]";

        flushDic.SetString("data", strData);
        flushDic.SetString("#app_id", m_appid);
        timeb t1;
        ftime(&t1);
        flushDic.SetDateTime("#flush_time", t1.time, t1.millitm);

        string flushStr = TDJSONObject::ToJson(flushDic);

        bool result = m_httpSend.Send(flushDic);
        if (result == true) {
            shared_ptr<TAITask> deleteTask(new TASqiteDeleteTask(m_sqliteQueue,uuids,false));
            ThinkingdataTask::getMDataTaskQue()->PushTask(deleteTask);
        }
    }

    void TASqiteInsetTask::Stop() {
//        printf("TASqiteInsetTask::Stop");
    }
    void TASqiteDeleteTask::Stop() {
//        printf("TASqiteInsetTask::Stop");
    }
    void TANetworkTask::Stop() {
//        printf("TASqiteInsetTask::Stop");
    }

    TATaskQueue::TATaskQueue() :
        m_pThread(nullptr) {
    }

    TATaskQueue::~TATaskQueue() {
        if (m_pThread != nullptr) {
            m_pThread->join();
            delete m_pThread;
            m_pThread = nullptr;
        }
    }

    bool TATaskQueue::Start() {
        m_pThread = new(nothrow) thread(&TATaskQueue::ThreadCallBack, this);
        return (m_pThread != NULL);
    }

    void TATaskQueue::PushTask(const shared_ptr<TAITask> &task) {
        m_lock.lock();
        m_taskQue.push(task);
        m_lock.unlock();
    }

    void TATaskQueue::ThreadCallBack() {

        while (true) {
            m_lock.lock();
            if (m_taskQue.empty()) {
                m_lock.unlock();
                continue;
            }
            m_currentTask = m_taskQue.front();
            m_taskQue.pop();
            m_lock.unlock();

            if (m_currentTask == nullptr) {continue;}

            m_currentTask->DoTask();
            shared_ptr<TAITask> tmp = m_currentTask;
            {
                m_currentTask = nullptr;
            }
        }

    }

    static TATaskQueue *m_ta_dataTaskQue;
    static TATaskQueue *m_ta_networkTaskQue;

    TATaskQueue *ThinkingdataTask::getMDataTaskQue() {
        if (m_ta_dataTaskQue != nullptr) {
            return m_ta_dataTaskQue;
        } else {
            m_ta_dataTaskQue = new TATaskQueue();
            m_ta_dataTaskQue->Start();
            return m_ta_dataTaskQue;
        }
    }

    TATaskQueue *ThinkingdataTask::getMNetworkTaskQue() {
        if (m_ta_dataTaskQue != nullptr) {
            return m_ta_dataTaskQue;
        } else {
            m_ta_dataTaskQue = new TATaskQueue();
            m_ta_dataTaskQue->Start();
            return m_ta_dataTaskQue;
        }
    }
};