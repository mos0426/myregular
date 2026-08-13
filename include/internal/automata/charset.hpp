#pragma once

#include <cstdint>
#include <map>
#include <vector>


enum class EndpointType: uint8_t{
    // 表示区间端点属于终点还是起点， 区间的表示形式为 [start, end)，即左闭右开区间
    START = 0,
    END = 1
};


class CharSet {
public:
    CharSet() = default;
    ~CharSet() = default;

    CharSet(uint32_t start, uint32_t end){
        endpoints_.insert({start, EndpointType::START});
        endpoints_.insert({end, EndpointType::END});
    }

    // 检查给定的码点是否在字符集范围内
    bool contains(uint32_t codepoint) const;

    // 增加一个码点区间，码点区间为左闭右开
    void add(uint32_t start, uint32_t end);

    // 禁止拷贝构造和拷贝赋值
    CharSet(const CharSet&) = delete;
    CharSet& operator=(const CharSet&) = delete;

    // 允许移动构造和移动赋值
    CharSet(CharSet&&) noexcept = default;
    CharSet& operator=(CharSet&&) noexcept = default;

private:
    // 使用 std::map 来存储字符范围的端点，key 为码点，value 为端点类型（起点或终点）
    std::map<uint32_t, EndpointType> endpoints_; 
};