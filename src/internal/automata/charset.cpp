#include <cstdint>
#include <map>
#include <vector>

#include "charset.hpp"


bool CharSet::contains(uint32_t codepoint) const {
    // 检查给定的码点是否在字符集范围内
    // 使用 std::map 的 upper_bound 方法查找第一个大于 codepoint 的端点
    auto it = endpoints_.upper_bound(codepoint);
    if (it == endpoints_.begin()){
        // 如果没有比 codepoint 更大的端点，说明 codepoint 小于所有端点，不在任何范围内
        return false;
    }
    --it; // 回退到小于等于 codepoint 的最大端点
    return it->second == EndpointType::START; // 如果该端点是 START，则 codepoint 在某个范围内
}


void CharSet::add(uint32_t start, uint32_t end){
    // 添加一个码点到字符集范围中

    // 处理 start, start 的端点类型是 EndpointType::START
    auto it = endpoints_.lower_bound(start);
    // 已存在码点值等于 start 的情况
    if (it->first == start) {
        // 端点类型为终点，则端点和 start 抵消
        if (it->second == EndpointType::END) endpoints_.erase(it);
        // 端点类型为起点, 则不做任何处理
    }
    else if (it == endpoints_.begin() || it->second == EndpointType::END){
        endpoints_.insert(it, {start, EndpointType::START});
    }
    
    // 处理 end
    it = endpoints_.lower_bound(end);
    // 已存在码点值等于 end 的情况
    if (it->first == end) {
        // 端点类型为起点， 则端点和 end 抵消
        if (it->second == EndpointType::START) endpoints_.erase(it);
        // 端点类型为终点，则不做任何处理
    }
    else{
        it ++; // it 转换成 upp_bound
        if (it == endpoints_.end() || it->second == EndpointType::START) {
            endpoints_.insert(--it, {end, EndpointType::END});
        }
    }
}


