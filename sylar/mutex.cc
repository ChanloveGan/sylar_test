#include "mutex.h"
// #include "macro.h"
// #include "scheduler.h"
#include <iostream>


namespace sylar {

Semaphore::Semaphore(uint32_t count) {
    /**
     * sem_init()
     * 功能：初始化无名信号量，也即在内存中创建一个未命名的信号量，并设置其初始值。
     * 输入参数：
     *     &m_semaphore：指向待初始化的信号量对象的指针（需预先分配内存）。
     *     0：信号量在线程间共享（默认）；非0表示信号量在进程间共享（需将 m_semaphore 放在共享内存中）
     *     count：信号量的初始值（必须 ≥ 0）
     * 输出参数：
     *     若创建成功，返回0
     *     若创建失败，返回-1，并设置 errno 表示具体错误
     */
    if(sem_init(&m_semaphore, 0, count)) {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore() {
    /**
     * sem_destroy()
     * 功能：销毁无名信号量（匿名信号量）​
     * 输入参数：
     *     &m_semaphore：指向待初始化的信号量对象的指针（需预先分配内存）。
     */
    sem_destroy(&m_semaphore);
}

void Semaphore::wait() {
    /**
     * sem_wait()
     * 功能：​信号量原子减一（P操作）​，尝试减少信号量的值。若信号量值 ​> 0，立即减1，函数返回。
     *      若信号量值 = 0，阻塞调用线程/进程，直到信号量值变为正数（由其他线程/进程通过sem_post()增加）。
     * 输入参数：
     *     &m_semaphore：已初始化的信号量对象的指针
     * 输出参数：
     *     若操作成功，返回0
     *     若操作失败，返回-1，并设置 errno 表示具体错误
     */
    if(sem_wait(&m_semaphore)) {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify() {
    /**
     * sem_post()
     * 功能：原子地增加信号量的值​（即“释放资源”）
     * 输入参数：
     *     &m_semaphore：已初始化的信号量对象的指针
     * 输出参数：
     *     若操作成功，返回0
     *     若操作失败，返回-1，并设置 errno 表示具体错误
     */
    if(sem_post(&m_semaphore)) {
        throw std::logic_error("sem_post error");
    }
}

FiberSemaphore::FiberSemaphore(size_t initial_concurrency)
    :m_concurrency(initial_concurrency) {
}

FiberSemaphore::~FiberSemaphore() {
    // SYLAR_ASSERT(m_waiters.empty());
}

bool FiberSemaphore::tryWait() {
    // SYLAR_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_concurrency > 0u) {
            --m_concurrency;
            return true;
        }
        return false;
    }
}

void FiberSemaphore::wait() {
    // SYLAR_ASSERT(Scheduler::GetThis());
    {
        MutexType::Lock lock(m_mutex);
        if(m_concurrency > 0u) {
            --m_concurrency;
            return;
        }
        // m_waiters.push_back(std::make_pair(Scheduler::GetThis(), Fiber::GetThis()));
    }
    // Fiber::YieldToHold();
}

void FiberSemaphore::notify() {
    MutexType::Lock lock(m_mutex);
    // if(!m_waiters.empty()) {
    //     auto next = m_waiters.front();
    //     m_waiters.pop_front();
    //     next.first->schedule(next.second);
    // } else {
    //     ++m_concurrency;
    // }
}

}
