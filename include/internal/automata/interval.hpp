#pragma once

#include <cstdint>
#include <limits>


// CODEPOINT_MIN 和 CODEPOINT_MAX 分别别是 UTF-8 编码字符的无穷小和无穷大
constexpr uint32_t CODEPOINT_MIN = std::numeric_limits<uint32_t>::min();
constexpr uint32_t CODEPOINT_MAX = std::numeric_limits<uint32_t>::max();


struct Interval{
    // 码点区间, 表示区间范围内的所有码点
    // 左闭右开
    uint32_t start;
    uint32_t end;   
};


struct DFAInterval{
    // 在确定性（DFA）有限自动机中，对于同一状态下的不同输入字符，如果它们跳转到的下一个状态相同，
    // 则这些字符可以被合并为一个连续的区间。本结构体即用于记录这一映射关系。
    // 区间为左闭右开
    uint32_t start;
    uint32_t end;
    size_t target_state;
};