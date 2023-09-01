//
// Created by wwango on 2022/11/14.
//

#ifndef UNTITLED1_TA_CPP_SEND_H
#define UNTITLED1_TA_CPP_SEND_H

#include <iostream>
#include <queue>

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

#if defined(_WIN32)
#define TD_MUTEX CRITICAL_SECTION
#define TD_MUTEX_LOCK(mutex) EnterCriticalSection((mutex))
#define TD_MUTEX_UNLOCK(mutex) LeaveCriticalSection((mutex))
#define TD_MUTEX_INIT(mutex) InitializeCriticalSection((mutex))
#define TD_MUTEX_DESTROY(mutex) DeleteCriticalSection((mutex))
#else
#define TD_MUTEX pthread_mutex_t
#define TD_MUTEX_LOCK(mutex) pthread_mutex_lock((mutex))
#define TD_MUTEX_UNLOCK(mutex) pthread_mutex_unlock((mutex))
#define TD_MUTEX_INIT(mutex) \
do { \
pthread_mutexattr_t mutex_attr; \
pthread_mutexattr_init(&mutex_attr); \
pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE); \
pthread_mutex_init((mutex), &mutex_attr); \
} while(0)
#define TD_MUTEX_DESTROY(mutex) pthread_mutex_destroy((mutex))
#endif

namespace thinkingdata {

    class TDJSONObject;

    using namespace std;

    class HttpSender {
    public:
        HttpSender(const string &server_url,
                   const string &appid);

        bool send(const string &data);

    private:
        static bool gzipString(const string &str,
                               string *out_string,
                               int compression_level);

        static bool encodeToRequestBody(const string &data,
                                        string *request_body);

        static string base64Encode(const string &data);

        friend class ThinkingAnalyticsAPI;

        static const int kRequestTimeoutSecond = 3;
        string server_url_;
        string appid_;
    };


    class TAHttpSend {
    public:
        TAHttpSend(const string &server_url,
                        const string &appid);

        void Init();

        bool Send(const TDJSONObject &record);

        ~TAHttpSend();

    private:

        static const size_t kFlushAllBatchSize = 30;

        TD_MUTEX records_mutex_;
        TD_MUTEX sending_mutex_;

        class LockGuard {
        public:
            LockGuard(TD_MUTEX *mutex) : mutex_(mutex) {
                TD_MUTEX_LOCK(mutex_);
            }

            ~LockGuard() {
                TD_MUTEX_UNLOCK(mutex_);
            }

        private:
            TD_MUTEX *mutex_;
        };

        std::deque <string> records_;
        string data_file_path_;
        HttpSender *sender_;
    };



};

#endif //UNTITLED1_TA_CPP_SEND_H
