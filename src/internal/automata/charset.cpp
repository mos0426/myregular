#include <cstdint>
#include <vector>
#include <cassert>
#include <utility>
#include <algorithm>

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


        if (first.empty() || second.empty()) return std::vector<Endpoint>();
        std::vector<Endpoint> new_endpoint_set;
        auto first_it = first.begin(), second_it = second.begin();
        // 记录已经遇到过的起点的个数，遇到起点计数 +1, 遇到终点计数 -1
        int counter = 0; 

        while (true){
            if (first_it->codepoint < second_it->codepoint){
                if (first_it->type == EndpointType::START){
                    if (counter > 0) new_endpoint_set.push_back(*first_it);
                    ++counter, ++first_it;
                }
                else{ // first_it->type == EndpointType::END
                    if (counter > 1) new_endpoint_set.push_back(*first_it);
                    --counter, ++first_it;
                    if (first_it == first.end()){
                        break;
                    } 
                }
            }
            else if (first_it->codepoint > second_it->codepoint) { 
                 if (second_it->type == EndpointType::START){
                    if (counter > 0) new_endpoint_set.push_back(*second_it);
                    ++counter, ++second_it;
                }
                else{ // second_it->type == EndpointType::END
                    if (counter > 1) new_endpoint_set.push_back(*second_it);
                    --counter, ++second_it;
                    if (second_it == second.end()){
                        break;
                    }
                }               
            }
            else{ // first_it->codepoint == second_it->codepoint
                if (first_it->type == EndpointType::START && second_it->type == EndpointType::START){
                    new_endpoint_set.push_back(*first_it);
                    counter += 2;
                    ++first_it, ++second_it;
                }
                else{
                    if (first_it->type != second_it->type) ;// 两个端点抵消
                    else{ // first_it->type == EndpointType::END && second_it->type == EndpointType::END
                        new_endpoint_set.push_back(*first_it);
                        counter -= 2;
                    }
                    ++first_it, ++second_it;
                    if (first_it == first.end() || second_it == second.end()) break;
                }
            }
        }

        return new_endpoint_set;
    }


    namespace{
        void endpoint_set_difference_re(
            std::vector<Endpoint>::const_iterator first_it,
            std::vector<Endpoint>::const_iterator first_end,
            std::vector<Endpoint>::const_iterator second_it,
            std::vector<Endpoint>::const_iterator second_end,
            std::vector<Endpoint> &new_set
        ){
            auto first_range_start = first_it, first_range_end = first_it + 1;
            auto second_range_end = second_it + 1;

            // second_range_end 迭代至第一个 codpoint 大于 first_range_start 的端点
            while (second_range_end->codepoint <= first_range_start->codepoint){
                second_range_end += 2;
                if (second_range_end == second_end){
                    for (; first_it != first_end; ++first_it) new_set.push_back(*first_it);
                    return ;
                } 
            }
            auto second_range_start = second_range_end - 1;
            if (second_range_start->codepoint >= first_range_end->codepoint){
                new_set.push_back(*first_range_start);
                new_set.push_back(*first_range_end);
                first_it = first_range_end + 1;
                if (first_it == first_end) return ;
                second_it = second_range_end - 1;
                return endpoint_set_difference_re(
                    first_it, first_end, second_it, second_end, new_set
                );
            }


            if (second_range_start->codepoint > first_range_start->codepoint){
                new_set.push_back(*first_range_start);
                new_set.emplace_back(Endpoint{second_range_start->codepoint, EndpointType::END});
            }
            new_set.emplace_back(Endpoint{second_range_end->codepoint, EndpointType::START});

            second_range_start += 2;
            if (second_range_start == second_end){
                ++first_it; // first_it = first_range_end
                 for (; first_it != first_end; ++first_it) new_set.push_back(*first_it);
                return ;
            }
            second_range_end += 2;

            while (second_range_end->codepoint < first_range_end->codepoint){
                new_set.emplace_back(Endpoint{second_range_start->codepoint, EndpointType::END});
                new_set.emplace_back(Endpoint{second_range_end->codepoint, EndpointType::START});
                second_range_start += 2;
                if (second_range_start == second_end){
                    ++first_it; // first_it = first_range_end
                    for (; first_it != first_end; ++first_it) new_set.push_back(*first_it);
                    return ;
                }
                second_range_end += 2;
            }

            if (second_range_start->codepoint < first_range_end->codepoint){
                new_set.emplace_back(Endpoint{second_range_start->codepoint, EndpointType::END});
            }
            else{ // second_range_start->codepoint >= first_range_end->codepoint
                new_set.push_back(*first_range_end);
            }

            first_it += 2;
            if (first_it == first_end) return ;

            return endpoint_set_difference_re(
                first_it, first_end, second_it, second_end, new_set
            );
        }
        
    }


    std::vector<Endpoint> endpoint_set_difference(
        const std::vector<Endpoint> &first,
        const std::vector<Endpoint> &second
    ){
        // 端点序列 first 对另一端点序列 second 求差集 (返回新端点序列 new_endpoint_set).
        // 输入 first 和 second 都是按 codepoint 升序排序的端点序列.
        // 复杂度:
        //  - 时间复杂度 O(n + m)，空间复杂度 O(n + m)，其中 n = first.size(), m = second.size().

        if (first.empty()) return std::vector<Endpoint>();
        if (second.empty()) return std::vector<Endpoint>(first);

        std::vector<Endpoint> new_endpoint_set;
        endpoint_set_difference_re(
            first.begin(), first.end(), second.begin(), second.end(), new_endpoint_set
        );
        return new_endpoint_set;
    }

    std::vector<Endpoint> endpoint_set_union(const std::vector<Endpoint> &first, const std::vector<Endpoint> &second){
        //将两个端点编码的区间集合合并为它们的并集（返回新的端点序列 new_endpoint_set）.
        //输入 first 和 second 都是按 codepoint 升序排序的端点序列.
        //复杂度:
        //  - 时间复杂度 O(n + m)，空间复杂度 O(n + m)，其中 n = first.size(), m = second.size().

        if (first.empty()) return std::vector<Endpoint>(second);
        if (second.empty()) return std::vector<Endpoint>(first);
        std::vector<Endpoint> new_endpoint_set;
        int counter = 0; // 记录 new_endpoint_set 中多余的 start 端点
        auto first_it = first.begin(), second_it = second.begin();

        while (true){
            if (first_it->codepoint < second_it->codepoint){
                if (first_it->type == EndpointType::START){
                    // 没有多余的起点
                    if (counter == 0) new_endpoint_set.push_back(*first_it);
                    counter += 1;
                    first_it++;
                }
                else{ // it->tye == EndpointType::END
                    // 只有一个多余的起点
                    if (counter == 1) new_endpoint_set.push_back(*first_it);
                    counter -= 1;
                    first_it++;
                    if (first_it == first.end()){
                        for (; second_it != second.end(); second_it++) new_endpoint_set.push_back(*second_it);
                        break;
                    }
                }
            }
            else if (first_it->codepoint > second_it->codepoint){
                if (second_it->type == EndpointType::START){
                    // 没有多余的起点
                    if (counter == 0) new_endpoint_set.push_back(*second_it);
                    counter += 1;
                    ++second_it;
                }
                else{ // it->tye == EndpointType::END
                    // 只有一个多余的起点
                    if (counter == 1) new_endpoint_set.push_back(*second_it);
                    counter -= 1;
                    ++second_it;
                    if (second_it == second.end()){
                        for (; first_it != first.end(); ++first_it) new_endpoint_set.push_back(*first_it);
                        break;
                    }
                }
            }
            else{ // first_it->codepoint == second_it->codepoint
                if (first_it->type == EndpointType::START && second_it->type == EndpointType::START){
                    if (counter == 0) new_endpoint_set.push_back(*first_it);
                    counter += 2;
                    ++first_it, ++second_it;
                }
                else{
                    // first_it->type == EndpointType::END && second_it->type == EndpointType::END
                    if (first_it->type == second_it->type){ 
                        if (counter == 2) new_endpoint_set.push_back(*first_it);
                        counter -= 2;
                    }
                    // first_it->type != second_it->type 
                    // 两个端点抵消， new_endpoint_set 不更新
                    ++first_it, second_it++;
                    if (first_it == first.end()){
                        for (; second_it != second.end(); ++second_it) new_endpoint_set.push_back(*second_it);
                        break;
                    }
                    if (second_it == second.end()){
                        for (; first_it != first.end(); ++first_it) new_endpoint_set.push_back(*first_it);
                        break;
                    }
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



bool check_endpoint_set(const std::vector<Endpoint> endpoint_set){
    if (endpoint_set.empty()) return true;
    
    auto current = endpoint_set.begin();
    if (current->type != EndpointType::START) return false;

    auto next = endpoint_set.begin() + 1;
    while (next != endpoint_set.end()){
        if (current->codepoint >= next->codepoint) return false;
        else{
            if (current->type == next->type) return false;
        };
        current++, next++;
    }

    if (current->type != EndpointType::END) return false;
    return true;
}


CharSet CharSet::unite(const CharSet &other) const {

    if (!negated_ && !other.negated_){
        return CharSet(endpoint_set_union(endpoint_set_, other.endpoint_set_), false);
    }
    else if (!negated_ && other.negated_){
        return CharSet(endpoint_set_difference(other.endpoint_set_, endpoint_set_), true);
    }
    else if (negated_ && !other.negated_){
        return CharSet(endpoint_set_difference(endpoint_set_, other.endpoint_set_), true);
    }
    else{ // negated_ && other.negated_
        return CharSet(endpoint_set_intersection(endpoint_set_, other.endpoint_set_), true);
    }
}


void CharSet::unite_update(uint32_t start, uint32_t end){
    if (negated_) *this = unite(CharSet(start, end));
    else{
        if (endpoint_set_.empty()){
            endpoint_set_.emplace_back(Endpoint{start, EndpointType::START});
            endpoint_set_.emplace_back(Endpoint{start, EndpointType::END});
        }
        else{
            if (start > (endpoint_set_.end()-1)->codepoint){
                endpoint_set_.emplace_back(Endpoint{start, EndpointType::START});
                endpoint_set_.emplace_back(Endpoint{end, EndpointType::END});
            }
            else *this = unite(CharSet(start, end));
        }
    }
};


