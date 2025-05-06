#include "util.h"
#include <execinfo.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
// #include <google/protobuf/unknown_field_set.h>

#include "log.h"
#include "fiber.h"
#include <pthread.h>
#include <sys/syscall.h>

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
    // return sylar::Fiber::GetFiberId();
    return sylar::Fiber::GetFiberId();
}

static std::string demangle(const char* str) {
    size_t size = 0;
    int status = 0;
    std::string rt;
    rt.resize(256);
    if(1 == sscanf(str, "%*[^(]%*[^_]%255[^)+]", &rt[0])) {
        // sscanf提取符号修饰名
        char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
        // 将符号修饰名转换为可读的原始函数/类型名
        if(v) {
            std::string result(v);
            free(v);
            return result;
        }
    }
    if(1 == sscanf(str, "%255s", &rt[0])) {
        return rt;
    }
    return str;
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void**)malloc((sizeof(void*) * size));
    size_t s = ::backtrace(array, size);
    // 将调用栈的返回地址按层级顺序填充至array，最多填充size个，返回实际填充的个数s
    char** strings = backtrace_symbols(array, s);
    // 将调用的函数名等信息转化为符号修饰名等信息
    if(strings == NULL) {
        SYLAR_LOG_ERROR(g_logger) << "backtrace_synbols error";
        free(strings);
        free(array);
        return;
    }

    for(size_t i = skip; i < s; ++i) {
        bt.push_back(demangle(strings[i]));
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

}
