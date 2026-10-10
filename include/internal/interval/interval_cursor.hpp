#pragma once

#include <memory>

#include "interval.hpp"


class IntervalCursor{
    // 管理区间集 (interval set)    
public:
    IntervalCursor() = default;

    IntervalCursor(const IntervalSet &intervalset, size_t label);

    Endpoint next();

    Endpoint current(){return current_;};

    size_t label(){return label_;};

    bool more(){return current_it_ != intervalset_ptr_->end();};

    friend bool operator<(IntervalCursor &left, IntervalCursor &right);

    // 拷贝构造
    IntervalCursor(const IntervalCursor &other) noexcept{
        intervalset_ptr_ = other.intervalset_ptr_;
        current_it_ = other.current_it_;
        current_ = other.current_;
        label_ = other.label_;
    };

    // 拷贝赋值
    IntervalCursor &operator=(const IntervalCursor &other) noexcept{
        if (this == &other) return *this;
        intervalset_ptr_ = other.intervalset_ptr_;
        current_it_ = other.current_it_;
        current_ = other.current_;
        label_ = other.label_;
        return *this;
    };

    // 移动构造
    IntervalCursor(IntervalCursor &&other) noexcept{
        intervalset_ptr_ = std::move(other.intervalset_ptr_);
        current_it_ = std::move(other.current_it_);
        current_ = std::move(other.current_);
        label_ = std::move(other.label_);
    };

    // 移动赋值
    IntervalCursor &operator=(IntervalCursor &&other) noexcept{
        if (this == &other) return *this;
        intervalset_ptr_ = std::move(other.intervalset_ptr_);
        current_it_ = std::move(other.current_it_);
        current_ = std::move(other.current_);
        label_ = std::move(other.label_);
        return *this;
    };

    //析构函数
    ~IntervalCursor() = default;


private:
    std::shared_ptr<const IntervalSet> intervalset_ptr_;
    IntervalSet::const_iterator current_it_;
    Endpoint current_;
    size_t label_;
};


// 按照 left.current.endpoint 和 right.current.endpoint 来对比
bool operator<(IntervalCursor &left, IntervalCursor &right);
