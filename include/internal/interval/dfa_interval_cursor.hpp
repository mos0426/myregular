#pragma once

#include "interval.hpp"


class DFAIntervalCursor{
    // 管理 DFA 区间集 (DFA interval set) 上的端点，按照正序输出端点流；
public:
    DFAIntervalCursor() = default;
    DFAIntervalCursor(const DFAIntervalSet &dfa_intervalset);

    Endpoint next();

    Endpoint current(){return current_;};

    size_t label(){return current_it_->target_state;};

    bool more(){return current_it_ != dfa_intervalset_->end();};

    friend bool operator<(DFAIntervalCursor &left, DFAIntervalCursor &right);

private:
    const DFAIntervalSet *dfa_intervalset_;
    DFAIntervalSet::const_iterator current_it_;
    Endpoint current_; 
};


// 按照 left.current.endpoint 和 right.current.endpoint 来对比
bool operator<(DFAIntervalCursor &left, DFAIntervalCursor &right);