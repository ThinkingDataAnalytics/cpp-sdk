//
// Created by DELL on 2023/6/27.
//
#include "ta_flush_task.h"
#include "ta_analytics_sdk.h"
#include "ta_cpp_helper.h"

namespace thinkingdata {
    TDFlushTask::~TDFlushTask() {
        std::unique_lock<std::mutex> lock(m_lock);
        is_stop = true;
        cv.notify_all();
        lock.unlock();
        if (m_pThread != nullptr) {
            if (m_pThread->joinable())
            {
                m_pThread->join();
            }
            delete m_pThread;
            m_pThread = nullptr;
        }
    }

    TDFlushTask::TDFlushTask() {

    }

    bool TDFlushTask::Start() {
        m_pThread = new(nothrow) thread(&TDFlushTask::ThreadCallBack, this);
        return (m_pThread != NULL);
    }

    void TDFlushTask::ThreadCallBack() {
        while (!is_stop){
            std::chrono::seconds timer_duration(ta_cpp_helper::flush_interval);
            std::unique_lock<std::mutex> lock(m_lock);
            cv.wait_for(lock, timer_duration, [this] { return is_stop; });
            ThinkingAnalyticsAPI::Flush();
            lock.unlock();
        }
    }
}