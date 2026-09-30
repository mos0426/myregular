#pragma once

#include <cstdint>
#include <limits>
#include <vector>


// CODEPOINT_MIN 和 CODEPOINT_MAX 分别别是 UTF-8 编码字符的无穷小和无穷大
constexpr uint32_t CODEPOINT_MIN = std::numeric_limits<uint32_t>::min();
constexpr uint32_t CODEPOINT_MAX = std::numeric_limits<uint32_t>::max();


struct Interval{
    // 码点区间, 表示区间范围内的所有码点
    // 左闭右开
    uint32_t start;
    uint32_t end;   
};


struct Endpoint{
    // 码点区间的端点
    uint32_t codepoint;
    bool is_start;
};


// 码点区间集，具体表现可视作在数轴上码点区间的正序排列；
// 码点区间集内不会存在有两个或多个区间重合的情况 (包括相交，包含，首尾相接)
using IntervalSet = std::vector<Interval>;


// 检查 interval set 的合法性
// 即 interval set 要符合区间起点小于终点, 正序排列，无重合区间三个特性
bool check_intervalset(const IntervalSet &intervalset);