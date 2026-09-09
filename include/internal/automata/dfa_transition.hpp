#pragma once

#include <vector>
#include <cstdint>
#include <limits>


constexpr uint32_t CODEPOINT_MIN = std::numeric_limits<uint32_t>::min();
constexpr uint32_t CODEPOINT_MAX = std::numeric_limits<uint32_t>::max();


struct DFAInterval{
    // 在确定性有限自动机中，对于同一状态下的不同输入字符，如果它们跳转到的下一个状态相同，
    // 则这些字符可以被合并为一个连续的区间。本结构体即用于记录这一映射关系。
    // 区间为左闭右开
    uint32_t start;
    uint32_t end;
    size_t target_state;
};


class DFATransition{
    // DFA 的状态转移函数
    // 对应一个输入状态的所有转移的聚合
    // '0' 为死状态
public:
    DFATransition() = default;
    ~DFATransition() = default;

    // 根据输入码点返回转移结果
    size_t next_state(uint32_t codepoint);

    // 添加一个原始的“码点区间 → 目标状态”映射
    // 使用此接口构建 DFA 时, 需要确保构造过程的码点区间对目标状态映射不会有冲突，此接口不会对冲突的码点区间进行正确性检查
    // 增加的码点区间和已有的码点区间产生重叠时，会合并重叠的码点区间，并将它们的目标状态设置为相同的目标状态
    // 两个码点区间在不同目标状态的情况允许终点和起点重叠
    // 例如，已有的码点区间为 [1, 3) → 2, [5, 8) → 3, 当调用 add_interval(2, 6, 4) 时，最终的码点区间为 [1, 8) → 4
    // 码点区间为左闭右开
    void add_interval(uint32_t start, uint32_t end, size_t target_state);

    // 允许拷贝构造和拷贝赋值
    DFATransition(const DFATransition&) = default;
    DFATransition& operator=(const DFATransition&) = default;

    // 允许移动构造和移动赋值
    DFATransition(DFATransition&&) noexcept = default;
    DFATransition& operator=(DFATransition&&) noexcept = default;
private:
    std::vector<DFAInterval> intervals_;
};