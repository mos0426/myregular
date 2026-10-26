#pragma once

#include "interval.hpp"

// 求两个区间集 (interval set) 的并集;
// 输入的区间集不能为空集：
IntervalSet interval_set_union(const IntervalSet &first, const IntervalSet &second);