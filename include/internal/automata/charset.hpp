#pragma once

#include <cstdint>
#include <vector>


enum class EndpointType: uint8_t{
    // 表示区间端点属于终点还是起点， 区间的表示形式为 [start, end)，即左闭右开区间
    START = 0,
    END = 1
};


struct Endpoint{
    // 码点区间端点
    uint32_t codepoint;
    EndpointType type;
};


class CharSet {
    // 字符集
    // 用码点区间的形式表示，所有的码点区间均为左闭右开
public:
    CharSet() = default;
    ~CharSet() = default;

    CharSet(uint32_t start, uint32_t end){
        endpoint_set_.push_back({start, EndpointType::START});
        endpoint_set_.push_back({end, EndpointType::END});
    }

    // 检查给定的码点是否在字符集范围内
    bool contains(uint32_t codepoint) const;

    // 增加一个码点区间
    void add(uint32_t start, uint32_t end);

    // 禁止拷贝构造和拷贝赋值
    CharSet(const CharSet&) = delete;
    CharSet& operator=(const CharSet&) = delete;

    // 允许移动构造和移动赋值
    CharSet(CharSet&&) noexcept = default;
    CharSet& operator=(CharSet&&) noexcept = default;

private:
    // 使用 std::map 来存储字符范围的端点，key 为码点，value 为端点类型（起点或终点）
    std::vector<Endpoint> endpoint_set_; 
    // 取反标志，若为 ture, 指定的字符集范围即为 endpoints_ 描述的字符集范围的补集
    bool negated_ = false;
};