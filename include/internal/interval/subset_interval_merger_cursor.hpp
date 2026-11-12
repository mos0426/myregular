#pragma once

#include <utility>
#include <vector>

#include "interval.hpp"
#include "cursor_merger.hpp"


class SubsetIntervalMergerCursor{
// 归并多个 SubsetIntervalSet;
// next() 按照区间的正序推出新 SubIntervalSet 的区间.
public:
    SubsetIntervalMergerCursor() = delete;
    SubsetIntervalMergerCursor(SubsetIntervalCursorMerger &merger):
    merger_(merger), refcount_({}), current_end_(0), has_more_(true) 
    {
        assert(merger.more());
        buffer_ = SubsetInterval{0, 0, {}};
        next(); // 把 buffer_ 内填入第一个区间.
    };

    // 由 merger_ 获取正序的端点流，处理端点流并更新 refcount_, 根据 refcount_ 的更新情况判断是否出现新区间
    // 并返回新区间;
    // 出现起点端点时，端点内的目标状态集 (endpoint.target_state_set) 在 refcount_ 的内对应的计数 +1, 出现
    // refcount_ 未计数的元素插入对应的状态的一次计数;
    // 出现终点端点时, 端点内的目标状态集 (endpoint.target_state_set) 在 refcount_ 的内对应的计数 -1, 理论
    // 上来说, 此时不会存在状态在 refcount_ 上未有计数;
    SubsetInterval next();

    inline bool more() const {return has_more_;};

private:
    SubsetIntervalCursorMerger &merger_;
    // refcount_ 用于反向目标状态的计数;
    // refcount_ 在所有操作过程中保持按照反向目标状态的数值大小正序排列.
    std::vector<std::pair<size_t, int>> refcount_;
    // 记录当前区间的起点;
    uint32_t current_start_;
    // 记录已经出现的可能的区间终点, 如果未出现可能的的区间终点, current_end_ = 0
    uint32_t current_end_;
    // 目前接收的端点的码点位置;
    uint32_t current_;
    // 生成的区间的缓存, next() 生成新区间时，对照缓存, 判断区间是否可合并(首尾相接, 目标状态集相等),
    // 可合并时会先合并, 然后继续找新区间，直到新区间不能和 buffer_ 合并;
    SubsetInterval buffer_;
    bool has_more_;

    SubsetInterval push_subset_interval() const;
    
    // 若 buffer_ 和 other 首尾相接并且目标状态集相等，合并 buffer_ 和 other 并把合并结果更新 buffer_, 返回 true;
    // 否则, 返回 false;
    bool merger_buffer(SubsetInterval &other);

    // 更新 refcount, 若 is_start 为 true, 计数 +1 , 若 is_start 为 false , 计数 -1 ;
    // 返回需要插入计数的记录;
    // 返回结果记录需要插入元素和需要插入 refcount 位置的索引;
    // pair 的第一个元素为 需要插入的元素, 第二个元素为需要插入 refcount_ 位置的索引;
    // 需要插入的元素后续插入的位置在记录的 refcount 的插入位置的后面;
    std::vector<std::pair<size_t, size_t>> refresh_refcount(const std::vector<size_t> &state_set, bool is_start);
    
    void insert_refcount(const std::vector<std::pair<size_t, size_t>> &insert_record);

    // 清除 refcount_ 内计数为 0 的元素.
    void clear_zero_refcount();

    // 检查 refcount_ 是否存在计数为 0 的元素.
    inline bool check_zero_refcount() const {
        for (auto c: refcount_){
            if (c.second == 0) return true;
        }
        return false;
    }
};
