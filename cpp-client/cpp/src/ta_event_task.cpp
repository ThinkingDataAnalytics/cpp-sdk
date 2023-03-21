//
// Created by wwango on 2022/11/10.
//

#include "ta_event_task.h"
#include <sys/timeb.h>
#include "ta_sqlite.h"
#include "ta_json_object.h"

namespace thinkingdata {

    mutex ta_sqlite_mtx;
  


    TASqiteInsetTask::TASqiteInsetTask(TAHttpSend& httpSend, TASqliteDataQueue& sqliteQueue, string event, string appid, bool enableLog, void (*callback)(int)) :m_httpSend(httpSend), m_sqliteQueue(sqliteQueue), m_appid(appid), m_event(event), m_enableLog(enableLog), m_callback(callback) {}
    TASqiteDeleteTask::TASqiteDeleteTask(TASqliteDataQueue &sqliteQueue, vector<string> uuids, bool enableLog): m_sqliteQueue(sqliteQueue), m_uuids(uuids), m_enableLog(enableLog) {}
    TANetworkTask::TANetworkTask(TASqliteDataQueue &sqliteQueue,TAHttpSend &httpSend, string appid, bool enableLog): m_sqliteQueue(sqliteQueue), m_httpSend(httpSend), m_appid(appid), m_enableLog(enableLog) {}

    TASqiteInsetTask::~TASqiteInsetTask() {
        // printf("\n[thinkingdata] TASqiteInsetTask deinit");
        //cout << "\n[thinkingdata] TASqiteInsetTask deinit" << endl; 
        /*int b = 123;
        wchar_t a[MAX_PATH] = { 0 };
        wsprintf(a, L"%d TASqiteInsetTask deinit\n", b);
        OutputDebugString(a);*/
    }
    TASqiteDeleteTask::~TASqiteDeleteTask() {
        // printf("\n[thinkingdata] TASqiteDeleteTask deinit");
        //cout << "\n[thinkingdata] TASqiteDeleteTask deinit" << endl;
        //int b = 123;
        //wchar_t a[MAX_PATH] = { 0 };
        //wsprintf(a, L"%d TASqiteDeleteTask deinit\n", b);
        //OutputDebugString(a);
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

void TASqiteInsetTask::DoTask() {
    try {
        if (m_enableLog == true) {
//            scanf_s("\n[ThinkingEngine] data queue: %s\n", m_event.c_str());
            cout << "\n[ThinkingEngine] data queue: " << m_event.c_str() << endl;
        }

    

        if (m_sqliteQueue.isStop) {
            return;
        }
        ta_sqlite_mtx.lock();
        long messageCount = m_sqliteQueue.addObject(m_event, m_appid);
        ta_sqlite_mtx.unlock();
        if (m_callback)
        {
            m_callback(messageCount);
        }
        //if (messageCount >= 20) {
        //    printf("\n[ThinkingEngine] Flush messageCount: %ld\n", messageCount);
        //    shared_ptr<TAITask> networkTask(new TANetworkTask(m_sqliteQueue, m_httpSend, m_appid, m_enableLog));
        //    ThinkingdataTask::getMNetworkTaskQue()->PushTask(networkTask);
        //}

    }
    catch (exception e) {
        
    }
}

    void TASqiteDeleteTask::DoTask() {
        try {
            if (m_sqliteQueue.isStop) {
                return;
            }
            ta_sqlite_mtx.lock();
            m_sqliteQueue.removeData(m_uuids);
           
            ta_sqlite_mtx.unlock();

        }
        catch (exception e) {

        }
        
    }

    void TANetworkTask::DoTask() {
        try{
            if (m_sqliteQueue.isStop) {
                return;
            }
            TDJSONObject flushDic;
           
            ta_sqlite_mtx.lock();
            vector<tuple<string, string>> records = m_sqliteQueue.getFirstRecords(10, m_appid);
            ta_sqlite_mtx.unlock();
        
                
            if (records.empty()) {
                return;
            }

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
                ta_sqlite_mtx.unlock();
                //shared_ptr<TAITask> deleteTask(new TASqiteDeleteTask(m_sqliteQueue, uuids, false));
               // ThinkingdataTask::getMDataTaskQue()->PushTask(deleteTask);
                //ThinkingdataTask::getMDataTaskQue()->cv.notify_one();
            }
          
            
        }
        catch (exception e) {
            
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
        isStop = true;
        if (m_pThread != nullptr) {
            m_pThread->join();
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

        while (true && !isStop) {

            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            if (!m_taskQue.empty()) {
                m_lock.lock();
                m_currentTask = m_taskQue.front();
                m_taskQue.pop();
                m_lock.unlock();

                if (m_currentTask == nullptr) {continue;}
                if (!isStop) {
                    m_currentTask->DoTask();
                }
                shared_ptr<TAITask> tmp = m_currentTask;
                {
                    m_currentTask = nullptr;
                }
            }else
            {
                
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
};