#pragma once

#include <vector>

// 判断状态集 second 是否被状态集 first 包含
// 输入的状态集必须无重复元素且按照正序排列
bool state_set_include(const std::vector<size_t> &first, const std::vector<size_t> &second);

// 判断状态 state 是否被状态集 state_set 包含
inline bool state_set_contain(const std::vector<size_t> &state_set, size_t state){
    for (auto s: state_set) if (s == state) return true;
    return false;
}


// 计算 first 和 second 的并集并返回
std::vector<size_t> state_set_union(const std::vector<size_t> &first, const std::vector<size_t> &second);


struct StateSetHash {
    // State Set 的哈希函数，使用 Boost 风格的 hash_combine 算法。
    // state 具体表现为正序排列且无重复元素的 std::vector<size_t>
    size_t operator()(const std::vector<size_t> &states) const {
        size_t h = 0;
        for (size_t s : states) {
            h ^= std::hash<size_t>{}(s) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};