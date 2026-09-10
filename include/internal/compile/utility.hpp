#pragma once

#include <vector>

// 判断状态集 second 是否被状态集 first 包含
// 输入的状态集必须无重复元素且按照正序排列
bool state_set_include(const std::vector<size_t> &first, const std::vector<size_t> &second);

// 判断状态 state 是否被状态集 state_set 包含
bool state_set_contain(const std::vector<size_t> &state_set, size_t state){
    for (auto s: state_set) if (s == state) return true;
    return false;
}