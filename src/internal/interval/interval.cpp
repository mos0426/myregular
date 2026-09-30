


#include "interval.hpp"

bool check_intervalset(const IntervalSet &intervalset){
    if (intervalset.size() <= 2){
        return true;
    }
    auto it = intervalset.begin();
    if (it->start >= it->end) return false;
    auto pre_it = it + 1;
    while (pre_it != intervalset.end()){
        if (pre_it->start >= pre_it->end) return false;
        if (it->end >= pre_it->start) return false; 
    }
    return true;
}