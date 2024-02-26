#include "ta_calibrated_time.h"
#include "ta_analytics_sdk.h"
#include "ta_cpp_helper.h"

#if defined(_WIN32)
#include <windows.h>
#include <locale>
#include <codecvt>
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif
namespace thinkingdata {
    void TDSystemInfo::enableTimeCalibrated(int64_t &currentTime){
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
    void TDSystemInfo::getTime(timeb *t) {
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

    TDSystemInfo::TDSystemInfo() {
        try{
            presetProperties.SetString("#lib_version", TD_LIB_VERSION);
            presetProperties.SetString("#lib", TD_LIB_NAME);
            presetProperties.SetString("#device_id", ta_cpp_helper::getDeviceID());
#if defined(_WIN32)
            presetProperties.SetString("#os", "Windows");
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            presetProperties.SetNumber("#screen_width",screenWidth);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            presetProperties.SetNumber("#screen_height",screenHeight);
            //get system language
//            LANGID language = GetSystemDefaultUILanguage();
            wchar_t buffer[LOCALE_NAME_MAX_LENGTH];
            if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SISO639LANGNAME, buffer, LOCALE_NAME_MAX_LENGTH)) {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::string languageStr = converter.to_bytes(buffer);
                presetProperties.SetString("#system_language",languageStr);
            }
            //get system zone offset
            TIME_ZONE_INFORMATION timeZoneInfo;
            DWORD result = GetTimeZoneInformation(&timeZoneInfo);
            if (result != TIME_ZONE_ID_INVALID) {
                int tzRet = timeZoneInfo.Bias / (-60);
                presetProperties.SetNumber("#zone_offset",tzRet);
            }
#elif defined(__APPLE__)
            presetProperties.SetString("#os", "Mac");
#endif
        }catch (const std::exception&){

        }
    }

}