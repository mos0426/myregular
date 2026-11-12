#include <vector>
#include <cassert>
#include <utility>

#include "subset_interval_merger_cursor.hpp"
#include "cursor_merger.hpp"
#include "interval.hpp"


SubsetInterval SubsetIntervalMergerCursor::next(){
    assert(more());

    if (!merger_.more()){
        has_more_ = false;
        return buffer_;
    }

    auto [endpoint, _] = merger_.next();
    
    // 确保每次都把 merger_ 重合的端点推出来，下一个 next() 得到得端点和上一个 next() 的最末尾的
    // 端点不重合;
    if (endpoint.is_start){

        // refcount_ 为空, endpoint 是新区间的起点.
        if (refcount_.empty()){
            current_start_ = endpoint.codepoint;
            assert(current_end_ == 0); // 确定未出现新区间终点
            insert_refcount(refresh_refcount(endpoint.target_state_set, true));
            // 处理所有和 endpoint 重合的端点.
            while (merger_.get_next().first.codepoint == endpoint.codepoint){
                auto [e, __] = merger_.next();
                insert_refcount(refresh_refcount(e.target_state_set, true));
            }
            // 新区间终点未出现, 继续看下一个端点.
            return next();
        }
        // refcount 不为空, endpoint 是新区间的终点, 是下一个区间的起点.
        else{ // !refcount_.empty()
            assert(current_end_ == 0); // 确定未出现新区间终点
            auto insert_record = refresh_refcount(endpoint.target_state_set, true);

            // 未产生新的计数, 跳过目前端点，继续看下一个端点.
            if (insert_record.empty()){
                return next();
            }

            // 产生新的计数, 出现新的区间.
            current_end_ = endpoint.codepoint;
            auto new_interval = push_subset_interval();
            current_start_ = endpoint.codepoint;
            current_end_ = 0;
            insert_refcount(insert_record);

            // 处理所有和 endpoint 重合的端点.
            while (merger_.get_next().first.codepoint == endpoint.codepoint){
                auto [e, __] = merger_.next();
                insert_refcount(refresh_refcount(e.target_state_set, true));
            }
            if (merger_buffer(new_interval)){
                return next();
            }
            else{
                auto result = std::move(buffer_);
                buffer_ = std::move(new_interval);
                return result;
            }
        }
    }
    else{ // !endpoint.is_start
        assert(!refcount_.empty());
        assert(current_end_ == 0);
        refresh_refcount(endpoint.target_state_set, false);
        // 处理端点类型为终点的重合端点.
        while (
            merger_.more() &&
            !merger_.get_next().first.is_start && 
            merger_.get_next().first.codepoint == endpoint.codepoint
        ){
            auto [e, __] = merger_.next();
            refresh_refcount(e.target_state_set, false);
        }

        // 如果 refcount_ 出现计数为 0 的元素, 那就意味着出现新区间;
        // 相反则未出现新区间, 跳过当前端点, 继续看下一个端点.
        if (check_zero_refcount()){
            current_end_ = endpoint.codepoint;
            auto new_interval = push_subset_interval();
            clear_zero_refcount();
            current_start_ = endpoint.codepoint;
            current_end_ = 0;
            // 处理端点类型为起点的重合端点;
            // 先前已经处理完所有端点类型为终点的重合端点, 所以后续出现的所有重合端点的类型都是起点.
            while (
                merger_.more() &&
                merger_.get_next().first.codepoint == endpoint.codepoint
            ){
                auto [e, __] = merger_.next();
                insert_refcount(refresh_refcount(e.target_state_set, true));
            }
            if (merger_buffer(new_interval)){
                return next();
            }
            auto result = std::move(buffer_);
            buffer_ = std::move(new_interval);
            return result;
        }
        else{ // !check_zero_refcount()
            return next();
        }

    }

    
};


SubsetInterval SubsetIntervalMergerCursor::push_subset_interval() const {
    std::vector<size_t> target_state_set;
    target_state_set.reserve(refcount_.size());
    for (auto c: refcount_){
        target_state_set.push_back(c.first);
    }
    return SubsetInterval(current_start_, current_end_, std::move(target_state_set));
}


bool SubsetIntervalMergerCursor::merger_buffer(SubsetInterval &other){
    
    // 判断是否首尾相接
    if (buffer_.end == other.start){
        // 判断目标状态集是否相等
        if (buffer_.target_state_set == other.target_state_set){
            buffer_.end = other.end;
            return true;
        }
    }
    return false;
}


std::vector<std::pair<size_t, size_t>> SubsetIntervalMergerCursor::refresh_refcount(
    const std::vector<size_t> &state_set, bool is_start
){
    if (is_start){
        // i 是 state_set 的索引, j 是 refcount_ 的索引
        int i = 0, j = 0;
        // 记录需要插入元素和需要插入 refcount_ 位置的索引;
        // pair 的第一个元素为 需要插入的元素, 第二个元素为需要插入 refcount_ 位置的索引;
        // 需要插入的元素后续插入的位置在记录的 refcount 的插入位置的前面;
        std::vector<std::pair<size_t, size_t>> insert_record;
        while (i < state_set.size() && j < refcount_.size()){
            if (state_set[i] < refcount_[j].first){
                insert_record.emplace_back(std::make_pair(state_set[i], j));
                ++i;
            }
            else if (state_set[i] > refcount_[j].first){
                ++j;
            }
            else{ // state_set[i] == refcount_[j].first
                ++refcount_[j].second;
                ++i, ++j;
            }
        }
        for (; i < state_set.size(); ++i){
            insert_record.emplace_back(std::make_pair(state_set[i], refcount_.size()));
        }
        return insert_record;
    }
    else{ // !is_start
        auto it1 = state_set.begin();
        auto it2 = refcount_.begin();
        while (it1 != state_set.end() && it2 != refcount_.end()){
            if (*it1 == it2->first){
                --it2->second;
                assert(it2->second >= 0);
                ++it1, ++it2;
            }
            else if (*it1 > it2->first){
                ++it2;
            }
            else{
                // 正常来说不可能出现 state_set 内的元素在 refcount_ 内没有计数;
                assert(false);
            }
        }
        assert(it1 == state_set.end() || it2 == refcount_.end());
        return {};
    }
};


void SubsetIntervalMergerCursor::insert_refcount(const std::vector<std::pair<size_t, size_t>> &insert_record){
    auto l_idx = refcount_.size()-1;
    // 记录 refcount_ 原来的大小 
    auto original_size = refcount_.size();
    refcount_.resize(refcount_.size() + insert_record.size());
    int n = insert_record.size() - 1;
    auto r_idx = refcount_.size() - 1;

    // 插入点最末尾的情况.
    while (n >= 0 && insert_record[n].second == original_size){
        refcount_[r_idx] = std::make_pair(insert_record[n].first, 1);
        --r_idx;
        --n;
        assert(r_idx >= 0);
    }

    while (l_idx >= 0 && n >= 0){
        if (l_idx == insert_record[n].second){
            refcount_[r_idx] = refcount_[l_idx];
            --r_idx, --l_idx;
            while (n >= 0 && l_idx == insert_record[n].second-1){
                refcount_[r_idx] = std::make_pair(insert_record[n].first, 1);
                --r_idx, --n;
            }
        }
        else{ // l_idx != insert_record[n].second
            refcount_[r_idx] = refcount_[l_idx];
            --r_idx, --l_idx;
        }
    }
}


void SubsetIntervalMergerCursor::clear_zero_refcount(){
    auto it = refcount_.begin();
    while (it != refcount_.end()){
        if (it->second != 0) ++it;
        else{
            // 需要删除的元素的起点位置
            auto it2 = it;
            ++it;
            while (it != refcount_.end()){
                if (it->second != 0){
                    *it2 = *it;
                    ++it, ++it2;
                }
                else{
                    ++it;
                }
            }
            refcount_.erase(it2, refcount_.end());
            return ;
        }
    }
    return ;
}


