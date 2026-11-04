#include <cassert>

#include "subset_interval_cursor.hpp"
#include "interval.hpp"


SubsetIntervalCursor::SubsetIntervalCursor(const SubsetIntervalSet &subset_intervalset, size_t label):
    subset_intervalset_(&subset_intervalset), current_it_(subset_intervalset_->begin()), label_(label){
        current_ = SubsetEndpoint{current_it_->start, true, current_it_->target_state_set};
    }


SubsetEndpoint SubsetIntervalCursor::next(){
    assert(more());
    if (current_.is_start){
        current_ = SubsetEndpoint{current_it_->end, false, current_it_->target_state_set};
        ++current_it_;
    }
    else{ // !current_.is_start
        current_ = SubsetEndpoint{current_it_->start, true, current_it_->target_state_set};
    }
    return current_;
}


bool operator<(SubsetIntervalCursor &left, SubsetIntervalCursor &right){
    if (left.current_.codepoint < right.current_.codepoint) return true;
        else if (left.current_.codepoint == right.current_.codepoint){
            // 码点值相同时， 终点比起点更大
            return !left.current_.is_start && right.current_.is_start;
        }
        return false;
};