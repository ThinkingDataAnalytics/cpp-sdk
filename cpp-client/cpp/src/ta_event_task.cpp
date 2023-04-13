//
// Created by wwango on 2022/11/10.
//

#include "ta_event_task.h"
#include <sys/timeb.h>
#include "ta_sqlite.h"
#include "ta_json_object.h"
#include "ta_cpp_utils.h"

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
        if (TAEnableLog::getEnableLog()) {
            cout << "\n[ThinkingEngine] data queue: " << m_event.c_str() << endl;
        }

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
                std::cout << "\n[ThinkingEngine] Failed to allocate memory for TANetworkTask Init" << std::endl;
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
        records = m_sqliteQueue->getFirstRecords(50, m_appid);
        ta_sqlite_mtx.unlock();

        while (!records.empty()) {

            vector<string> data;
            vector<string> uuids;
            string strData = "[";
            for (auto record : records) {
                uuids.push_back(get<0>(record));
                strData += get<1>(record);
                strData += ",";
            }
            strData = strData.substr(0, strData.size() - 1);
            strData += "]";

            flushDic.SetString("data", strData);
            flushDic.SetString("#app_id", m_appid);
            timeb t1;
            ftime(&t1);
            flushDic.SetDateTime("#flush_time", t1.time, t1.millitm);

            string flushStr = TDJSONObject::ToJson(flushDic);

            bool result = m_httpSend->Send(flushDic);
            if (result == true) {
                ta_sqlite_mtx.lock();
                m_sqliteQueue->removeData(uuids);
                records = m_sqliteQueue->getFirstRecords(50, m_appid);
                ta_sqlite_mtx.unlock();
            } else {
                break;
            }
        }
    }

    void TAFlushTask::DoTask() {
        if (TAEnableLog::getEnableLog()) {
            cout << "\n[ThinkingEngine] Flush Task"<< endl;
        }

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
            std::cout << "\n[ThinkingEngine] Failed to allocate memory for TANetworkTask Init" << std::endl;
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
        isStop = true;
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
        m_lock.lock();
        m_taskQue.push(task);
        m_lock.unlock();
    }

    void TATaskQueue::ThreadCallBack() {

        while (!isStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            m_lock.lock();
            if (!m_taskQue.empty()) {
                shared_ptr<TAITask> tmp = m_taskQue.front();
                if (!isStop && tmp != nullptr) {
                    tmp->DoTask();
                }
                m_taskQue.pop();
                tmp.reset();
            }
            m_lock.unlock();
        }
    }
};