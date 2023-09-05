//
// Created by DELL on 2023/6/27.
//

#ifndef UNTITLED1_TA_FLUSH_TASK_H
#define UNTITLED1_TA_FLUSH_TASK_H
#include <iostream>
#include <thread>
#include <condition_variable>
namespace thinkingdata{

    using namespace std;

    class TDFlushTask{
    public:
        ~TDFlushTask();
        TDFlushTask();
        bool Start();
        void ThreadCallBack();
    private:
        bool is_stop = false;
        condition_variable cv;
        mutex m_lock;
        thread *m_pThread;
    };
}

#endif //UNTITLED1_TA_FLUSH_TASK_H
