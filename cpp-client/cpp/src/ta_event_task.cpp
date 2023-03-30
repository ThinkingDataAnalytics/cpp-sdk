//
// Created by wwango on 2022/11/10.
//

#include "ta_event_task.h"
#include <sys/timeb.h>
#include "ta_sqlite.h"
#include "ta_json_object.h"
#include "ta_cpp_utils.h"

namespace thinkingdata {

    mutex ta_sqlite_mtx;

    TASqiteInsetTask::TASqiteInsetTask(TAHttpSend& httpSend, TASqliteDataQueue& sqliteQueue, string event, string appid) :m_httpSend(httpSend), m_sqliteQueue(sqliteQueue), m_appid(appid), m_event(event) {}
    TANetworkTask::TANetworkTask(TASqliteDataQueue &sqliteQueue,TAHttpSend &httpSend, string appid): m_sqliteQueue(sqliteQueue), m_httpSend(httpSend), m_appid(appid) {}
    TAFlushTask::TAFlushTask(TASqliteDataQueue &sqliteQueue,TAHttpSend &httpSend, string appid): m_sqliteQueue(sqliteQueue), m_httpSend(httpSend), m_appid(appid) {}

    TASqiteInsetTask::~TASqiteInsetTask() {
        // printf("\n[thinkingdata] TASqiteInsetTask deinit");
        //cout << "\n[thinkingdata] TASqiteInsetTask deinit" << endl;
        /*int b = 123;
        wchar_t a[MAX_PATH] = { 0 };
        wsprintf(a, L"%d TASqiteInsetTask deinit\n", b);
        OutputDebugString(a);*/
    }
    TANetworkTask::~TANetworkTask() {
        // printf("\n[thinkingdata] TANetworkTask deinit");
        // cout << "\n[thinkingdata] TANetworkTask deinit" << endl;
        
        //�ٳɹ����½��ַ����������ӡ����
        /*int b = 123;
        wchar_t a[MAX_PATH] = { 0 };
        wsprintf(a, L"%d TANetworkTask deinit\n", b);
        OutputDebugString(a);*/
    }
    TAFlushTask::~TAFlushTask() {
        // printf("\n[thinkingdata] TANetworkTask deinit");
        // cout << "\n[thinkingdata] TANetworkTask deinit" << endl;

        //�ٳɹ����½��ַ����������ӡ����
        /*int b = 123;
        wchar_t a[MAX_PATH] = { 0 };
        wsprintf(a, L"%d TANetworkTask deinit\n", b);
        OutputDebugString(a);*/
    }

    void TASqiteInsetTask::DoTask() {
        try {
            if (TAEnableLog::getEnableLog()) {
                cout << "\n[ThinkingEngine] data queue: " << m_event.c_str() << endl;
            }

            if (m_sqliteQueue.isStop) {
                return;
            }

            ta_sqlite_mtx.lock();
            long messageCount = m_sqliteQueue.addObject(m_event, m_appid);
            ta_sqlite_mtx.unlock();

            if (messageCount >= 30) {
                shared_ptr<TAITask> networkTask(new TANetworkTask(m_sqliteQueue, m_httpSend, m_appid));
                ThinkingdataTask::getMNetworkTaskQue()->PushTask(networkTask);
            }
        } catch (exception e) {

        }
    }

    void TANetworkTask::DoTask() {
        try{
            if (m_sqliteQueue.isStop) {
                return;
            }
            TDJSONObject flushDic;

            ta_sqlite_mtx.lock();
            vector<tuple<string, string>> records = m_sqliteQueue.getFirstRecords(50, m_appid);
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

                bool result = m_httpSend.Send(flushDic);
                if (result == true) {
                    ta_sqlite_mtx.lock();
                    m_sqliteQueue.removeData(uuids);
                    records = m_sqliteQueue.getFirstRecords(50, m_appid);
                    ta_sqlite_mtx.unlock();
                } else {
                    break;
                }
            }
        }
        catch (exception e) {
            
        }
        
    }

    void TAFlushTask::DoTask() {
        try {
            if (TAEnableLog::getEnableLog()) {
                cout << "\n[ThinkingEngine] Flush Task"<< endl;
            }

            if (m_sqliteQueue.isStop) {
                return;
            }

            shared_ptr<TAITask> networkTask(new TANetworkTask(m_sqliteQueue, m_httpSend, m_appid));
            ThinkingdataTask::getMNetworkTaskQue()->PushTask(networkTask);
        } catch (exception e) {

        }
    }

    void TASqiteInsetTask::Stop() {
//        printf("TASqiteInsetTask::Stop");
    }

    void TANetworkTask::Stop() {
//        printf("TASqiteInsetTask::Stop");
    }

    void TAFlushTask::Stop() {
//        printf("TASqiteInsetTask::Stop");
    }

    TATaskQueue::TATaskQueue() :
        m_pThread(nullptr) {
    }

    TATaskQueue::~TATaskQueue() {
        isStop = true;
        if (m_pThread != nullptr) {
            //cv.notify_one();
            if (m_pThread->joinable())
            {
                m_pThread->join();
            }
            delete m_pThread;
            m_pThread = nullptr;
        }

        while (!m_taskQue.empty()) {
            m_currentTask = m_taskQue.front();
            m_taskQue.pop();

            if (m_currentTask == nullptr) { continue; }
            shared_ptr<TAITask> tmp = m_currentTask;
            {
                m_currentTask = nullptr;
            }
           // m_currentTask.reset();
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
        //cv.notify_one();
    }

    void TATaskQueue::ThreadCallBack() {

        while (true && !isStop) {

             std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if (!m_taskQue.empty()) {
                m_lock.lock();
                m_currentTask = m_taskQue.front();
                m_taskQue.pop();
                m_lock.unlock();

                if (m_currentTask == nullptr) { continue; }
                if (!isStop) {
                    m_currentTask->DoTask();
                }
                shared_ptr<TAITask> tmp = m_currentTask;
                {
                    m_currentTask = nullptr;
                }
                //m_currentTask.reset();
            }
            /*else
            {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock);
            }*/
        }
    }

    static TATaskQueue *m_ta_dataTaskQue;
    static TATaskQueue *m_ta_networkTaskQue;

    TATaskQueue *ThinkingdataTask::getMDataTaskQue() {
        if (m_ta_dataTaskQue != nullptr) {
            return m_ta_dataTaskQue;
        } else {
            m_ta_dataTaskQue = new TATaskQueue();
            m_ta_dataTaskQue->isStop = false;
            m_ta_dataTaskQue->Start();
            return m_ta_dataTaskQue;
        }
    }

    TATaskQueue *ThinkingdataTask::getMNetworkTaskQue() {
        if (m_ta_networkTaskQue != nullptr) {
            return m_ta_networkTaskQue;
        }
        else {
            m_ta_networkTaskQue = new TATaskQueue();
            m_ta_networkTaskQue->isStop = false;
            m_ta_networkTaskQue->Start();
            return m_ta_networkTaskQue;
        }
    }
     /*int b = 123;
        wchar_t a[MAX_PATH] = { 0 };
        wsprintf(a, L"%d TASqiteInsetTask deinit\n", b);
        OutputDebugString(a);*/
};