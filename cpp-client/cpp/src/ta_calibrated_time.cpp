#include "ta_calibrated_time.h"
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif
namespace thinkingdata {
    void TDTimeCalibrated::enableTimeCalibrated(int64_t &currentTime){
        this->isCalibrated = true;
        this->currentTime = currentTime;
        #if defined(_WIN32)
        DWORD ticks = GetTickCount();
        this->systemTickCount =  static_cast<int64_t>(ticks);
        #elif defined(__APPLE__)
        uint64_t time = mach_absolute_time();
        // Get time base information
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);

        // Convert time to nanoseconds
        uint64_t nanoseconds = time * info.numer / info.denom;

        // convert time to milliseconds
        this->systemTickCount = static_cast<int64_t>(nanoseconds / 1000000);
        this->systemTickCount = (this->systemTickCount << 1) >> 1;
        #endif
    }
    void TDTimeCalibrated::getTime(timeb *t) {
        if(this->isCalibrated){
            #if defined(_WIN32)
            DWORD ticks = GetTickCount();
            int64_t t1 = static_cast<int64_t>(ticks);
            int64_t l = this->currentTime + t1 - this->systemTickCount;
            t->time = l/1000;
            t->millitm = l%1000;
            #elif defined(__APPLE__)
            uint64_t time = mach_absolute_time();
            mach_timebase_info_data_t info;
            mach_timebase_info(&info);
            uint64_t nanoseconds = time * info.numer / info.denom;
            int64_t t2 = static_cast<int64_t>(nanoseconds / 1000000);
            t2 = (t2 << 1) >> 1;
            int64_t l = this->currentTime + t2 - this->systemTickCount;
            t->time = l/1000;
            t->millitm = l%1000;
            #endif
        }else{
            //Time calibration is not enabled Get the current system time
            ftime(t);
        }
    }
}