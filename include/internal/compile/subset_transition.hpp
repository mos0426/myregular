#pragma once

#include <vector>
#include <cstdint>

#include "interval.hpp"
#include "charset.hpp"
#include "nfa.hpp"  


class SubsetTransition{
    // 子集构造法中，DFA 的一个状态对应 NFA 的一个状态集合。对于同一状态集合下的不同输入字符，如果它们跳转到的下一个状态集合相同，
    // 则这些字符可以被合并为一个连续的区间。本结构体即用于记录这一映射关系。
    // SubsetTransition 可以被看作在码点数轴上的码点区间的, 每个码点区间都可以映射到对应的状态集，如
    // [1, 3) → {2, 3, 4}, [3, 8) → {2, 5, 6}, [9, 10) → {2, 3, 4}, [11, 12) → {2, 5, 6}
    // 区间为左闭右开
public:
    SubsetTransition() = delete;
    ~SubsetTransition() = default;


    SubsetTransition(const std::vector<const NFATransition*> &nfa_transitions);

    // 允许拷贝构造和拷贝赋值
    SubsetTransition(const SubsetTransition&) = default;
    SubsetTransition& operator=(const SubsetTransition&) = default;

    // 允许移动构造和移动赋值
    SubsetTransition(SubsetTransition&&) noexcept = default;
    SubsetTransition& operator=(SubsetTransition&&) noexcept = default;
private:
    std::vector<SubsetInterval> intervals_;
};

