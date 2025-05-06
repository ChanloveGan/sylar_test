/**
 * @file util.h
 * @brief 常用的工具函数
 * @author sylar.yin
 * @email 564628276@qq.com
 * @date 2019-05-27
 * @copyright Copyright (c) 2019年 sylar.yin All rights reserved (www.sylar.top)
 */

#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__

#include <cxxabi.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <iomanip>
// #include <json/json.h>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
// #include <google/protobuf/message.h>
// #include "sylar/util/hash_util.h"
// #include "sylar/util/json_util.h"
// #include "sylar/util/crypto_util.h"


namespace sylar {

/**
 * @brief 返回当前线程的ID
 */
pid_t GetThreadId();

/**
 * @brief 返回当前协程的ID
 */
uint32_t GetFiberId();

/**
 * @brief 获取当前的调用栈
 * @param[out] bt 保存调用栈
 * @param[in] size 最多返回层数
 * @param[in] skip 跳过栈顶的层数
 */
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);

/**
 * @brief 获取当前栈信息的字符串
 * @param[in] size 栈的最大层数
 * @param[in] skip 跳过栈顶的层数
 * @param[in] prefix 栈信息前输出的内容
 */
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");


template<class T>
const char* TypeToName() {
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

// std::string PBToJsonString(const google::protobuf::Message& message);

// template<class Iter>
// std::string Join(Iter begin, Iter end, const std::string& tag) {
//     std::stringstream ss;
//     for(Iter it = begin; it != end; ++it) {
//         if(it != begin) {
//             ss << tag;
//         }
//         ss << *it;
//     }
//     return ss.str();
// }

//[begin, end)
//if rt > 0, 存在,返回对应index
//   rt < 0, 不存在,返回对于应该存在的-(index + 1)
template<class T>
int BinarySearch(const T* arr, int length, const T& v) {
    int m = 0;
    int begin = 0;
    int end = length - 1;
    while(begin <= end) {
        m = (begin + end) / 2;
        if(v < arr[m]) {
            end = m - 1;
        } else if(arr[m] < v) {
            begin = m + 1;
        } else {
            return m;
        }
    }
    return -begin - 1;
}

inline bool ReadFixFromStream(std::istream& is, char* data, const uint64_t& size) {
    uint64_t pos = 0;
    while(is && (pos < size)) {
        is.read(data + pos, size - pos);
        pos += is.gcount();
    }
    return pos == size;
}

template<class T>
bool ReadFromStream(std::istream& is, T& v) {
    return ReadFixFromStream(is, (char*)&v, sizeof(v));
}

template<class T>
bool ReadFromStream(std::istream& is, std::vector<T>& v) {
    return ReadFixFromStream(is, (char*)&v[0], sizeof(T) * v.size());
}

template<class T>
bool WriteToStream(std::ostream& os, const T& v) {
    if(!os) {
        return false;
    }
    os.write((const char*)&v, sizeof(T));
    return (bool)os;
}

template<class T>
bool WriteToStream(std::ostream& os, const std::vector<T>& v) {
    if(!os) {
        return false;
    }
    os.write((const char*)&v[0], sizeof(T) * v.size());
    return (bool)os;
}

class SpeedLimit {
public:
    typedef std::shared_ptr<SpeedLimit> ptr;
    SpeedLimit(uint32_t speed);
    void add(uint32_t v);
private:
    uint32_t m_speed;
    float m_countPerMS;

    uint32_t m_curCount;
    uint32_t m_curSec;
};

bool ReadFixFromStreamWithSpeed(std::istream& is, char* data,
                    const uint64_t& size, const uint64_t& speed = -1);

bool WriteFixToStreamWithSpeed(std::ostream& os, const char* data,
                            const uint64_t& size, const uint64_t& speed = -1);

template<class T>
bool WriteToStreamWithSpeed(std::ostream& os, const T& v,
                            const uint64_t& speed = -1) {
    if(os) {
        return WriteFixToStreamWithSpeed(os, (const char*)&v, sizeof(T), speed);
    }
    return false;
}

template<class T>
bool WriteToStreamWithSpeed(std::ostream& os, const std::vector<T>& v,
                            const uint64_t& speed = -1,
                            const uint64_t& min_duration_ms = 10) {
    if(os) {
        return WriteFixToStreamWithSpeed(os, (const char*)&v[0], sizeof(T) * v.size(), speed);
    }
    return false;
}

template<class T>
bool ReadFromStreamWithSpeed(std::istream& is, const std::vector<T>& v,
                            const uint64_t& speed = -1) {
    if(is) {
        return ReadFixFromStreamWithSpeed(is, (char*)&v[0], sizeof(T) * v.size(), speed);
    }
    return false;
}

template<class T>
bool ReadFromStreamWithSpeed(std::istream& is, const T& v,
                            const uint64_t& speed = -1) {
    if(is) {
        return ReadFixFromStreamWithSpeed(is, (char*)&v, sizeof(T), speed);
    }
    return false;
}

std::string Format(const char* fmt, ...);
std::string Formatv(const char* fmt, va_list ap);

template<class T>
void Slice(std::vector<std::vector<T> >& dst, const std::vector<T>& src, size_t size) {
    size_t left = src.size();
    size_t pos = 0;
    while(left > size) {
        std::vector<T> tmp;
        tmp.reserve(size);
        for(size_t i = 0; i < size; ++i) {
            tmp.push_back(src[pos + i]);
        }
        pos += size;
        left -= size;
        dst.push_back(tmp);
    }

    if(left > 0) {
        std::vector<T> tmp;
        tmp.reserve(left);
        for(size_t i = 0; i < left; ++i) {
            tmp.push_back(src[pos + i]);
        }
        dst.push_back(tmp);
    }
}


}

#endif
