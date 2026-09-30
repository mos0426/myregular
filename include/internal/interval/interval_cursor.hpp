
#include "interval.hpp"


class IntervalCursor{
    // 管理区间集 (interval set) 
public:
    IntervalCursor() = delete;

    IntervalCursor(const IntervalSet &intervalset, size_t label);

    Endpoint next();

    Endpoint current(){return current_;};

    size_t label(){return label_;};

    bool more(){return current_it_ != intervalset_.end();};

    friend bool operator<(IntervalCursor &left, IntervalCursor &right);


private:
    const IntervalSet &intervalset_;
    IntervalSet::const_iterator current_it_;
    Endpoint current_;
    size_t label_;
};


// 按照 left.current.endpoint 和 right.current.endpoint 来对比
bool operator<(IntervalCursor &left, IntervalCursor &right);
