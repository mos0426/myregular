#include <cstdint>
#include <vector>
#include <cassert>
#include <utility>

#include "charset.hpp"

namespace{

    struct CodepointRange{
        std::vector<Endpoint>::const_iterator start;
        std::vector<Endpoint>::const_iterator end;
    };

    std::vector<Endpoint> endpoint_set_intersection(
        const std::vector<Endpoint> &first,
        const std::vector<Endpoint> &second
    ){
        // 求端点序列 first 对另一端点序列 second 的差集 (返回新端点序列 new_endpoint_set).
        // 输入 first 和 second 都是按 codepoint 升序的端点序列.
        // 复杂度:
        //  - 时间复杂度 O(n + m)，空间复杂度 O(n + m)，其中 n = first.size(), m = second.size().

        // 按正序分别提取 fisrt 和 second 的码点区间 first_range 和 second_range 逐个作交集运算

        if (first.empty() || second.empty()) return std::vector<Endpoint>();
        std::vector<Endpoint> new_endpoint_set;
        auto first_it = first.begin(), second_it = second.begin();

        auto get_next_range = [](std::vector<Endpoint>::const_iterator &it) -> CodepointRange {
            CodepointRange result = CodepointRange{it, ++it};
            it++;
            return result;
        };

        auto codepoint_intersection = [&new_endpoint_set](const CodepointRange &left, const CodepointRange &right) -> void {
            // 对码点范围 left 和 right 求差集, 结果压入 new_endpoint_set
            // left.end->codepoint < right.end->codepoint
            if (left.start->codepoint >= right.start->codepoint){
                //区间 [*left.start, *right.end) 压入 new_endpoint_set
                new_endpoint_set.push_back(*left.start);
                new_endpoint_set.push_back(*right.end);
            }
            // left.start->codepint < right.start->codepoint
            else{
                if (left.end > right.start){
                    // 区间 [*right.start, *left.end) 压入 new_endpoint_set
                    new_endpoint_set.push_back(*right.start);
                    new_endpoint_set.push_back(*left.end);
                }
            }
        };

        CodepointRange first_range = get_next_range(first_it), second_range = get_next_range(second_it);

        while (true){

            // 判断 first_range 和 second_range 在码点数轴谁更靠右
            // 定义: 对比两个区间, 区间终点码点更大则为在码点数轴上更靠右
            // 靠左的区间对应的迭代器继续推进, 靠右区间的保留不动
            // 两个区间的终点码点值相等则对应的迭代器同时推进
            if (first_range.end->codepoint < second_range.end->codepoint){
                codepoint_intersection(first_range, second_range);
                if (first_it != first.end()) first_range = get_next_range(first_it);
                else break;
            }
            else if (first_range.end->codepoint > second_range.end->codepoint){
                codepoint_intersection(second_range, first_range);
                if (second_it != second.end()) second_range = get_next_range(second_it);
                else break;
            }
            // first_range.end->codepoint == second_range.end->codepoint
            else{
                codepoint_intersection(first_range, second_range);
                if ((first_it != first.end()) && (second_it != second.end())){
                    first_range = get_next_range(first_it);
                    second_range = get_next_range(second_it);
                }
                else break;
            }
        }

        return new_endpoint_set;
    }


    std::vector<Endpoint> endpoint_set_difference(
        const std::vector<Endpoint> &first,
        const std::vector<Endpoint> &second
    ){
        // 端点序列 first 对另一端点序列 second 求差集 (返回新端点序列 new_endpoint_set).
        // 输入 first 和 second 都是按 codepoint 升序排序的端点序列.
        // 复杂度:
        //  - 时间复杂度 O(n + m)，空间复杂度 O(n + m)，其中 n = first.size(), m = second.size().

        // 遍历 first 的码点区间 range, 逐个 range 对 second 求差集:{
        //  second_it 推进到 range 起点后.
        //  对比 second 当前端点和 range 起点码点值:
        //      - second 当前端点码点值大于 range 起点码点值:
        //          - second 当前端点类型 START, range 起点压入 new_endpoint_set， 创建新端点压入 new_endpoint_set
        //            新端点的码点值为 second 当前端点码点值, 端点类型为 END, 推进 second;
        //          - second 当前端点类型为 END, 创建新端点压入 new_endpoint_set, 新端点码点值为 second 当
        //            前端点码点值, 端点类型为 START, 推进 second;
        //      - 码点值相等:
        //          - second 当前端点类型为 START, second 当前端点和 range起点抵消, second 推进:
        //              - 对比 second 当前端点和 range 终点的码点值:
        //                  - second 当前端点码点值小于 range 终点码点值, 创建新端点压入 new_endpoint_set
        //                    新端点码点值为 second 当前端点码点值, 端点类型为 STAR, 推进 second;
        //                  - second 当前端点码点值大于等于 range 终点码点值, range 湮灭, 跳入下一循环.
        //          - second 当前端点类型为 END, range 起点压入 new_endpoint_set, 推进 second;
        //  遍历 second, 直到 second 推进到 码点值大于等于 range 终点值:{
        //      创建新端点压入 new_endpoint_set, 新端点码点值为 second 当前端点码点值, 端点类型于 second 当前端点类型相反,
        //      推进 second.
        //  }
        //  - second 当前端点码点值等于 range 终点码点值:
        //      - second 当前端点类型为 START, range 终点压入 new_endpoint_set;
        //      - second 当前端点类型为 END, 跳入下一循环;
        // - second 当前端点码点值大于 range 终点码点值, 跳入下一循环.
        // }

        if (first.empty()) return std::vector<Endpoint>();
        if (second.empty()) return std::vector<Endpoint>(first);

        std::vector<Endpoint> new_enpoint_set;
        auto first_it = first.begin(), second_it = second.begin();


        for (auto range_start = first_it, range_end = first_it + 1; first_it != first.end(); first_it = first_it+2){
            for (; second_it <= range_start; second_it++);  // second_it 推进到 range_start 后
            
            if (second_it->codepoint > range_start->codepoint){
                if (second_it->type == EndpointType::START){
                    new_enpoint_set.push_back(*range_start);
                    new_enpoint_set.emplace_back(Endpoint{second_it->codepoint, EndpointType::END});
                    second_it++;
                }
                else{
                    new_enpoint_set.emplace_back(Endpoint(second_it->codepoint, EndpointType::START));
                    second_it++;
                }
            }
            else if (second_it->codepoint == range_start->codepoint){
                if (second_it->type == EndpointType::START){
                    second_it++;  // 端点抵消
                    if (second_it->codepoint < range_end->codepoint){
                        new_enpoint_set.emplace_back(Endpoint{second_it->codepoint, EndpointType::START});
                        second_it++;
                    }
                    else continue;
                }
                else{
                    new_enpoint_set.push_back(*range_start);
                    second_it++;
                }
            }

            for (; second_it->codepoint < range_end->codepoint; second_it++){
                if (second_it->type == EndpointType::START){
                    new_enpoint_set.emplace_back(Endpoint{second_it->codepoint, EndpointType::END});
                }
                else new_enpoint_set.emplace_back(Endpoint{second_it->codepoint, EndpointType::START});
            }

            if (second_it->codepoint == range_end->codepoint){
                if (second_it->type == EndpointType::START){
                    new_enpoint_set.emplace_back(Endpoint{second_it->codepoint, EndpointType::END});
                }
            }
        }
    }


    std::vector<Endpoint> endpoint_set_union(const std::vector<Endpoint> &first, const std::vector<Endpoint> &second){
        //将两个端点编码的区间集合合并为它们的并集（返回新的端点序列 new_endpoint_set）.
        //输入 first 和 second 都是按 codepoint 升序排序的端点序列.
        //复杂度:
        //  - 时间复杂度 O(n + m)，空间复杂度 O(n + m)，其中 n = first.size(), m = second.size().

        //遍历 fisrt 和 second 端点，对比 first 和 second 端点的码点值{
        //根据对比结果，挑选其中一个端点 temp 作下一步处理或把两个端点抵消：
        //  - 若 first 和 second 当前端点码点值相等:
        //      - 若两个端点类型不同，则两个端点抵消, first 和 second 迭代器推进, 跳入下一循环;
        //      - 若两个端点类型相同, 则取任意其中一个端点作下一步处理.
        //  - 若 first 和 second 当前码点值不等, 取较小值并推进其迭代器.
        //对比 temp 和上一端点的类型:
        //  - 上一端点类型为 START:
        //      - temp 端点类型为 START, 跳入下一循环;
        //      - temp 端点类型为 END, temp 压入 new_endpoint_set.
        //  - 上一端点类型为 END:
        //      - temp 端点类型为 START, temp 压入 new_endpoint_set;
        //      - temp 端点类型为 END, 跳入下一循环.
        //}
        
        if (first.empty()) return std::vector<Endpoint>(second);
        if (second.empty()) return std::vector<Endpoint>(first);
        std::vector<Endpoint> new_endpoint_set;
        auto first_it = first.begin(), second_it = second.begin();

        if (first_it->codepoint < second_it->codepoint){
            new_endpoint_set.push_back(*first_it);
            first_it++;
        }
        else if (first_it->codepoint > second_it->codepoint){
            new_endpoint_set.push_back(*second_it);
            second_it++;
        }
        else{
            new_endpoint_set.push_back(*first_it);
            first_it++, second_it++;
        }
        assert(new_endpoint_set[0].type == EndpointType::START);

        while (true){
            Endpoint temp;
            if (first_it == first.end()){
                for(second_it; second_it != second.end(); second_it++) new_endpoint_set.push_back(*second_it);
                return new_endpoint_set;   
            }
            if (second_it == second.end()){
                for(first_it; first_it != first.end(); first_it++) new_endpoint_set.push_back(*first_it);
                return new_endpoint_set;
            }

            if (first_it->codepoint < second_it->codepoint){
                temp = *first_it;
                first_it++;
            }
            else if (first_it->codepoint > second_it->codepoint){
                temp = *second_it;
                second_it++;
            }
            else{
                if (first_it->type == second_it->type){
                    temp = *first_it;
                    first_it++, second_it++;
                }
                else{
                    first_it++, second_it++;
                    continue;
                }
            }

            if (new_endpoint_set.back().type == EndpointType::START){  // 对比前一个端点的类型
                if (temp.type == EndpointType::START) continue;
                else new_endpoint_set.push_back(temp);
            }
            else{
                if (temp.type == EndpointType::START) new_endpoint_set.push_back(temp);
                else {
                    new_endpoint_set.pop_back();
                    new_endpoint_set.push_back(temp);
                }
            }
        }

        return new_endpoint_set;
    }
}


bool CharSet::contains(uint32_t codepoint) const {
    // 检查给定的码点是否在字符集范围内
    bool is_contain;
    auto it = endpoint_set_.begin();
    for (;(it != endpoint_set_.end()) && (codepoint >= it->codepoint); it++); // 找到第一个码点值大于 codepoint 的端点
    if ((it == endpoint_set_.end()) || (it->type == EndpointType::START)) is_contain = false;
    else is_contain = true;
    return is_contain != negated_; 
}


void CharSet::add(uint32_t start, uint32_t end){
    // 添加一个码点到字符集范围中
    std::vector<Endpoint> a_range = {Endpoint{start, EndpointType::START}, Endpoint{end, EndpointType::END}};
    std::vector<Endpoint> temp;
    if (negated_) temp = endpoint_set_difference(endpoint_set_, a_range);
    else temp = endpoint_set_union(endpoint_set_, a_range);
    endpoint_set_.swap(temp);
}




