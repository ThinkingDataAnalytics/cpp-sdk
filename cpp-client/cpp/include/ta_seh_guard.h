//
// SEH 边界：把 SDK 自己线程上的执行、以及 Init/UnInit 里在调用方线程上同步执行的
// sqlite 操作包起来，硬件异常不再向上传播到宿主进程的未处理异常过滤器。
//

#ifndef UNTITLED1_TA_SEH_GUARD_H
#define UNTITLED1_TA_SEH_GUARD_H

namespace thinkingdata {

    /**
     * 捕获过一次硬件异常之后 SDK 永久停用，此后 tdSehCall 直接返回 false 且不执行 fn。
     * 这个 latch 不只是为了避免反复撞同一个坏状态：SEH 展开不会执行 C++ 析构，崩在
     * 临界区里的锁（ta_sqlite_mtx 等）永远不会被释放，只有保证之后没人再去拿这把锁
     * 才不会死锁。
     */
    bool tdSehTripped();

    /**
     * 在 SEH 保护下执行 fn(arg)。返回 false 表示发生了硬件异常，或 SDK 已经停用，
     * 两种情况调用方都必须按失败处理。
     *
     * fn 的栈帧在异常路径上不做 C++ 展开，局部对象不会析构，所以 fn 只能用在
     * 「出事即整体停用」的语义下，不能当常规错误处理用。
     * 栈溢出不拦（保护页已耗尽，吞掉没有安全保证），交给宿主。
     */
    bool tdSehCall(void (*fn)(void *), void *arg, const char *tag);
}

#endif //UNTITLED1_TA_SEH_GUARD_H
