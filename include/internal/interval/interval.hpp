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
    // Interval 码点区间的端点
    uint32_t codepoint;
    bool is_start;
};


struct DFAInterval{
    // 在确定性（DFA）有限自动机中，对于同一状态下的不同输入字符，如果它们跳转到的下一个状态相同，
    // 则这些字符可以被合并为一个连续的区间。本结构体即用于记录这一映射关系。
    // 区间为左闭右开
    uint32_t start;
    uint32_t end;
    size_t target_state;
};


struct DFAEndpoint{
    // DFAInterval 上的端点，带有区间上目标状态 (target state) 信息 
    uint32_t codepoint;
    bool is_start;
    size_t target_state;
};


struct SubsetInterval{
    // 在子集构造法中，DFA 的一个状态对应 NFA 的一个状态集合。对于同一状态集合下的不同输入字符，如果它们跳转到的下一个状态集合相同，
    // 则这些字符可以被合并为一个连续的区间。本结构体即用于记录这一映射关系。
    // 区间为左闭右开
    // 实际同样用在 dfa 最小化方法中用于构建反向转移表 (reverse transition table);
    uint32_t start;
    uint32_t end;
    std::vector<size_t> target_state_set;
};


// 码点区间集，具体表现可视作在数轴上码点区间的正序排列；
// 码点区间集内不会存在有两个或多个区间重合的情况 (包括相交，包含，首尾相接)
using IntervalSet = std::vector<Interval>;


// DFA 码点区间集，具体表现可视作数轴上码点区间的正序排列;
// 每个区间带有一个目标状态, 可以视为一个 DFA 上某个指定状态出发的所有转移边的聚合;
// 区间集内不会存在相交的区间，可能回出现首尾相接的情况；
using DFAIntervalSet = std::vector<DFAInterval>;


// 检查 interval set 的合法性
// 即 interval set 要符合区间起点小于终点, 正序排列，无重合区间三个特性
bool check_intervalset(const IntervalSet &intervalset);