#include <cassert>

#include "dfa_interval_cursor.hpp"


DFAIntervalCursor::DFAIntervalCursor(const DFAIntervalSet &dfa_intervalset, size_t label):
    dfa_intervalset_(&dfa_intervalset), current_it_(dfa_intervalset_->begin()), label_(label){
        current_ = DFAEndpoint{current_it_->start, true, current_it_->target_state};
    }


DFAEndpoint DFAIntervalCursor::next(){
    assert(more());
    if (current_.is_start){
        current_ = DFAEndpoint{current_it_->end, false, current_it_->target_state};
        ++current_it_;
    }
    else{ // !current_.is_start
        current_ = DFAEndpoint(current_it_->start, true, current_it_->target_state);
    }
    return current_;
}


bool operator<(DFAIntervalCursor &left, DFAIntervalCursor &right){
    if (left.current_.codepoint < right.current_.codepoint) return true;
        else if (left.current_.codepoint == right.current_.codepoint){
            // 码点值相同时， 终点比起点更大
            return !left.current_.is_start && right.current_.is_start;
        }
        return false;
};