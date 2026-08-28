//
// Created for SDK crash containment.
//

#include "ta_seh_guard.h"
#include "ta_cpp_helper.h"
#include <atomic>
#include <string>
#include <cstdio>

#if defined(_WIN32) && defined(_MSC_VER)
#include <windows.h>
#endif

namespace thinkingdata {

    static const int TD_SEH_CALLBACK_CODE = 1005;

    static std::atomic<bool> ta_seh_tripped(false);

    bool tdSehTripped() {
        return ta_seh_tripped.load();
    }

#if defined(_WIN32) && defined(_MSC_VER)

    static int tdSehFilter(unsigned long code) {
        if (code == EXCEPTION_STACK_OVERFLOW) {
            return EXCEPTION_CONTINUE_SEARCH;
        }
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // 独立函数：带 __except 的函数里不能出现需要析构的 C++ 对象（MSVC C2712）。
    static void tdSehReport(const char *tag, unsigned long code) {
        char codeText[16] = {0};
        sprintf_s(codeText, sizeof(codeText), "0x%08lX", code);
        std::string msg = std::string("Hardware exception ") + codeText + " caught in " +
                          (tag != nullptr ? tag : "unknown") + ", SDK is disabled";
        ta_cpp_helper::printSDKLog(TDLogLevel::TDERROR, msg);
        ta_cpp_helper::handleTECallback(TD_SEH_CALLBACK_CODE, msg);
    }

    // 上报本身也可能踩到同一片坏内存，再抛就没人接了，所以单独兜一层。
    static void tdSehReportGuarded(const char *tag, unsigned long code) {
        __try {
            tdSehReport(tag, code);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    bool tdSehCall(void (*fn)(void *), void *arg, const char *tag) {
        if (fn == nullptr || ta_seh_tripped.load()) {
            return false;
        }
        __try {
            fn(arg);
            return true;
        }
        __except (tdSehFilter(GetExceptionCode())) {
            ta_seh_tripped.store(true);
            tdSehReportGuarded(tag, GetExceptionCode());
            return false;
        }
    }

#else

    bool tdSehCall(void (*fn)(void *), void *arg, const char *tag) {
        (void) tag;
        if (fn == nullptr || ta_seh_tripped.load()) {
            return false;
        }
        fn(arg);
        return true;
    }

#endif
}
