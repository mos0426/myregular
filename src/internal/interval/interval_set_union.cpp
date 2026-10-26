#include <utility>
#include <cstdint>
#include <cassert>
#include <tuple>

#include "interval_set_union.hpp"
#include "interval.hpp"
#include "interval_cursor.hpp"
#include "cursor_merger.hpp"


IntervalSet interval_set_union(const IntervalSet &first, const IntervalSet &second){
    IntervalSet ans;
    
    IntervalCursorMerger merger = IntervalCursorMerger({
        IntervalCursor(first, 1), IntervalCursor(second, 2)
    });

    assert(merger.more());

    std::pair<int, int> refcount = {0, 0};
    auto [current_start, label] = merger.next();
    if (label == 1) ++refcount.first;
    else ++refcount.second;

    while (merger.more()){
        auto [endpoint, label] = merger.next();

        if (endpoint.is_start){
            if (label == 1) ++refcount.first;
            else ++refcount.second;
        }
        else{
            if (label == 1) --refcount.first;
            else --refcount.second;

            if (refcount.first == 0 && refcount.second == 0){

                // 检查下一个端点是否于目前端点重合
                if (merger.more()){
                    if (merger.get_next().first.codepoint != endpoint.codepoint){
                    ans.emplace_back(Interval{current_start.codepoint, endpoint.codepoint});
                    std::tie(current_start, label) = merger.next();
                    if (label == 1) ++refcount.first;
                    else ++refcount.second;
                    }
                }
                else ans.emplace_back(Interval{current_start.codepoint, endpoint.codepoint});
                
            }
        }
    }
    return ans;
}