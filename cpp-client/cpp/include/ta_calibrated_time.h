//
// Created by DELL on 2023/5/12.
//

#ifndef UNTITLED1_TA_CALIBRATED_TIME_H
#define UNTITLED1_TA_CALIBRATED_TIME_H
#include <sys/timeb.h>
#ifdef _MSC_VER
#if _MSC_VER >= 1600
#include <cstdint>
#else
typedef __int32 int32_t;
typedef __int64 int64_t;
#endif
#elif __GNUC__ >= 3
#include <cstdint>
#endif
namespace thinkingdata{
    class TDTimeCalibrated{
    public:
        bool isCalibrated = false;
        int64_t systemTickCount = 0;
        int64_t currentTime = 0;
        void enableTimeCalibrated(int64_t &currentTime);
        void getTime(timeb *t);
    };
}

#endif //UNTITLED1_TA_CALIBRATED_TIME_H
