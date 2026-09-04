#pragma once

#include <cstdint>
#include <vector>

#include "interval.hpp"


class CharSet {
    // 字符集
    // 用码点区间的形式表示，所有的码点区间均为左闭右开
public:
    CharSet() = default;
    ~CharSet() = default;

    CharSet(uint32_t start, uint32_t end):interval_set_({{start, end}}){};

    // 检查给定的码点是否在字符集范围内
    bool contains(uint32_t codepoint) const;

    // 增加一个码点区间
    // 针对连续正序添加区间的情况优化的接口，如 [[1, 3], [5, 8], [10, 20]....] 之类的
    void unite_update(uint32_t start, uint32_t end);
    void unite_update(const CharSet &other);

    // 增加一个通配符字符集
    void unite_wildcard_update(){
        interval_set_.clear();
        interval_set_.push_back({CODEPOINT_MIN, CODEPOINT_MAX});
    };

    // 取反字符集
    // 例如，字符集为 [1, 3), [5, 8) 时，取反后为:
    // [CODEPOINT_MIN, 1), [3, 5), [8, CODEPOINT_MAX)
    void negation_update();

    // 禁止拷贝构造和拷贝赋值
    CharSet(const CharSet&) = default;
    CharSet& operator=(const CharSet&) = default;

    // 允许移动构造和移动赋值
    CharSet(CharSet&&) noexcept = default;
    CharSet& operator=(CharSet&&) noexcept = default;

private:
    std::vector<Interval> interval_set_; 
};

