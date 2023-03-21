//
// Created by wwango on 2022/11/10.
//

#ifndef UNTITLED1_TA_EVENT_TASK_H
#define UNTITLED1_TA_EVENT_TASK_H

#include <iostream>
#include <functional>
#include <thread>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "ta_sqlite.h"
#include "ta_cpp_send.h"


namespace thinkingdata {

    class TAHttpSend;
    class TATaskQueue;

    using namespace std;

    class ThinkingdataTask {

    public:
        static TATaskQueue *getMDataTaskQue();
        static TATaskQueue *getMNetworkTaskQue();
    };

    class TAITask {
    public:
        virtual ~TAITask() {

        }
        virtual void DoTask() = 0;
        virtual void Stop() = 0;
    };

    class TASqiteInsetTask : public TAITask {
    public:
        ~TASqiteInsetTask();
        void DoTask();
        void Stop();

        TASqiteInsetTask(TAHttpSend& m_httpSend, TASqliteDataQueue &sqliteQueue, string event, string appid, bool enableLog, void(*callback)(int));
    private:
        string m_event;
        string m_appid;
        TASqliteDataQueue &m_sqliteQueue;
        bool m_enableLog;
        TAHttpSend& m_httpSend;
        void(*m_callback)(int);
    };

    class TASqiteDeleteTask : public TAITask {
    public:
        ~TASqiteDeleteTask();
        void DoTask();
        void Stop();
        TASqiteDeleteTask(TASqliteDataQueue &sqliteQueue, vector<string> uuids, bool enableLog);
    private:
        vector<string> m_uuids;
        TASqliteDataQueue &m_sqliteQueue;
        bool m_enableLog;
    };

    class TANetworkTask : public TAITask {
    public:
        ~TANetworkTask();
        void DoTask();
        void Stop();
        TANetworkTask(TASqliteDataQueue &sqliteQueue, TAHttpSend &httpSend, string appid, bool enableLog);
    private:
        TASqliteDataQueue &m_sqliteQueue;
        TAHttpSend &m_httpSend;
        string m_appid;
        bool m_enableLog;
    };


    class TATaskQueue {
    public:
        bool isStop;
        TATaskQueue();
        ~TATaskQueue();

    public:
        bool Start();
        void PushTask(const std::shared_ptr<TAITask> &task);
        void ThreadCallBack();

    private:
        mutex m_lock;
        thread *m_pThread;

        queue<shared_ptr<TAITask>> m_taskQue;
        shared_ptr<TAITask> m_currentTask;
    };

};
#endif //UNTITLED1_TA_EVENT_TASK_H
