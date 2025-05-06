#include "thread.h"
#include "log.h"
#include "util.h"

namespace sylar {

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}

void Thread::SetName(const std::string& name) {
    if(name.empty()) {
        return;
    }
    if(t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

Thread::Thread(std::function<void()> cb, const std::string& name)
    :m_cb(cb)
    ,m_name(name) {
    if(name.empty()) {
        m_name = "UNKNOW";
    }

    /**
     * pthread_create()
     * 功能：创建新线程
     * 输入参数：
     *     &m_thread：指向线程标识符的指针，m_thread表示线程标识符
     *     nullptr：设置线程属性（分离状态、调度策略）为默认
     *     &Thread::run：线程入口函数地址。新线程从该函数开始执行
     *     this：传递给Thread::run()函数的参数，这里是指向Thread的实例的指针
     * 输出参数：
     *     rt：
     *         若创建成功，返回0
     *         若创建失败，返回错误码
     */
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if(rt) {
        SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
            << " name=" << name;
        throw std::logic_error("pthread_create error");
    }
    m_semaphore.wait();
}

Thread::~Thread() {
    if(m_thread) {
            /**
         * pthread_detach()
         * 功能：将目标线程标记为“分离状态”，使其终止时自动释放资源。
         * 输入参数：
         *     m_thread：线程标识符
         */
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if(m_thread) {
        /**
         * pthread_join()
         * 功能：阻塞调用线程，直到目标线程 m_thread 结束执行，释放目标线程的内核资源（如线程描述符），接收目标线程的退出值。
         * 输入参数：
         *     m_thread：目标线程的标识符；
         *     nullptr：用于存储目标线程退出值的指针（若为 NULL，表示不接收返回值）。
         * 输出参数：
         *     rt：
         *         若成功，返回0
         *         若失败，返回错误码
         */
        int rt = pthread_join(m_thread, nullptr);
        if(rt) {
            SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->m_name;
    thread->m_id = sylar::GetThreadId();

    /**
     * pthread_setname_np()
     * 功能：设置线程名称
     * 输入参数：
     *     pthread_self()：要设置名称的线程的标识符（pthread_self返回当前调用线程的唯一标识符）
     *     thread->m_name.substr(0, 15).c_str()：所设置的线程名字，这里截取了前15个字符
     */
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);

    thread->m_semaphore.notify();

    cb();
    return 0;
}

}
