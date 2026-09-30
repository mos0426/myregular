#include <cassert>


#include "interval_cursor.hpp"


IntervalCursor::IntervalCursor(const IntervalSet &intervalset, size_t label):
    intervalset_(intervalset), current_it_(intervalset.begin()), label_(label){
        current_ = Endpoint{current_it_->start, true};
    }


Endpoint IntervalCursor::next(){
    assert(more());
    if (current_.is_start){
        current_ = Endpoint{current_it_->end, false};
        ++current_it_;
    }
    else{ // !current_.is_start
        current_ = Endpoint(current_it_->start, true);
    }
    return current_;
};


bool operator<(IntervalCursor &left, IntervalCursor &right){
    if (left.current_.codepoint < right.current_.codepoint) return true;
        else if (left.current_.codepoint == right.current_.codepoint){
            // 码点值相同时， 终点比起点更大
            return !left.current_.is_start && right.current_.is_start;
        }
        return false;
};