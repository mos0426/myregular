#pragma once

#include "interval.hpp"


class SubsetIntervalCursor{
    // 管理子集区间集 (Subset interval set) 上的端点，按照正序输出端点流;
public:
    SubsetIntervalCursor() = default;
    SubsetIntervalCursor(const SubsetIntervalSet &subset_intervalset, size_t label);

    SubsetEndpoint next();

    SubsetEndpoint current(){return current_;};

    size_t label(){return label_;};

    bool more(){return current_it_ != subset_intervalset_->end();};

    friend bool operator<(SubsetIntervalCursor &left, SubsetIntervalCursor &right);

private:
    const SubsetIntervalSet *subset_intervalset_;
    SubsetIntervalSet::const_iterator current_it_;
    SubsetEndpoint current_; 
    size_t label_;
};


// 按照 left.current.endpoint 和 right.current.endpoint 来对比
bool operator<(SubsetIntervalCursor &left, SubsetIntervalCursor &right);