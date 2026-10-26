#include <vector>
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <utility>
#include <tuple>
#include <unordered_map>
#include <queue>

#include "compile.hpp"
#include "interval.hpp"
#include "interval_cursor.hpp"
#include "cursor_merger.hpp"
#include "charset.hpp"
#include "utility.hpp"
#include "dfa.hpp"
#include "nfa.hpp"

namespace{
    class SubsetIntervalsCursor{
    public:
        SubsetIntervalsCursor() = delete;

        SubsetIntervalsCursor(std::vector<const NFATransition*> ps){
            assert(!ps.empty());

            std::vector<IntervalCursor> cursors;
            for (auto p: ps) {
                if (!p->char_set.empty()) cursors.emplace_back(IntervalCursor(p->char_set.get_intervals(), p->target_state));
            }

            merger_ = IntervalCursorMerger(cursors);

            if (!merger_.more()){
                has_more_ = false;
                return;
            }
            has_more_ = true;
            auto [endpoint, target] = merger_.next();
            assert(endpoint.is_start);
            current_start_ = endpoint.codepoint;
            refcount_.emplace_back(std::make_pair(target, 1));
            next();
        }
        
        std::tuple<uint32_t, uint32_t, std::vector<size_t>> next(){

            assert(has_more_);

            if (!merger_.more()){
                has_more_ = false;
                assert(refcount_.empty());
                return subset_interval_buffer_;
            }
            
            auto [endpoint, target_state] = merger_.next();
            // endpoint 与 current_start 重合的情况
            if (endpoint.codepoint == current_start_){
                auto it = refcount_.begin();
                if (endpoint.is_start){
                    while (it != refcount_.end()){
                        if (it->first >= target_state){
                            if (it->first == target_state){
                                ++(it->second);
                                return next();
                            }
                            refcount_.insert(it, {target_state, 1});
                            return next();
                        }
                        ++it;
                    }
                    refcount_.emplace_back(std::make_pair(target_state, 1));
                    return next();
                }
                else{ // !endpoint.is_start
                    while (it != refcount_.end()){
                        if (it->first == target_state){
                            --(it->second);
                            if (it->second == 0){
                                refcount_.erase(it);
                                // 判断是否走到尽头
                                if (refcount_.empty() && !merger_.more()){
                                    has_more_ = false;
                                    return subset_interval_buffer_;
                                }
                            }
                            return next();
                        }
                    }
                    // endpoint 为终点时，refcount 必然包含 target_state
                    assert(false);
                    return subset_interval_buffer_;
                }
            }

            if (endpoint.is_start){
                if (refcount_.empty()){
                    refcount_.emplace_back(std::make_pair(target_state, 1));
                    current_start_ = endpoint.codepoint;
                    return next();
                }
                auto it = refcount_.begin();
                while (it != refcount_.end()){
                    if (it->first >= target_state){
                        if (it->first == target_state){
                            ++(it->second);
                            return next();
                        }
                        auto result = subset_interval_buffer_;
                        push_interval(endpoint.codepoint);
                        refcount_.insert(it, {target_state, 1});
                        current_start_ = endpoint.codepoint;
                        return result;
                    }
                    ++it;
                }
                auto result = subset_interval_buffer_;
                push_interval(endpoint.codepoint);
                refcount_.emplace_back(std::make_pair(target_state, 1));
                current_start_ = endpoint.codepoint;
                return result;
            }
            else{ // !endpoint.is_start
                assert(!refcount_.empty());
                auto it = refcount_.begin();
                while (it != refcount_.end()){
                    if (it->first >= target_state){
                        if (it->first == target_state){
                            --(it->second);
                            if (it->second == 0){
                                auto result = subset_interval_buffer_;
                                push_interval(endpoint.codepoint);
                                refcount_.erase(it);
                                current_start_ = endpoint.codepoint;
                                return result;
                            }
                            return next();
                        }
                        // endpoint 为终点时，refcount 必然包含 target_state
                        assert(false);
                    }
                    ++it;
                }
                // endpoint 为终点时，refcount 必然包含 target_state
                assert(false);
                return subset_interval_buffer_;
            }
        };

        bool more(){return has_more_;};

    private:
        IntervalCursorMerger merger_;
        // 目前处理区间的起点
        uint32_t current_start_;
        // 用于记录目前处理区间的 target state 和每个 target state 出现的次数
        // refcount_ 按照 target state 的数值大小正序排列
        std::vector<std::pair<size_t, int>> refcount_;
        bool has_more_;
        std::tuple<uint32_t, uint32_t, std::vector<size_t>> subset_interval_buffer_;

        void push_interval(uint32_t end){
            assert(!refcount_.empty());
            std::vector<size_t> target_state_set;
            for (auto i: refcount_){
                target_state_set.push_back(i.first);
            }
            assert(current_start_ < end);  
            subset_interval_buffer_ = {current_start_, end, target_state_set};
            return ;
        }
    };


    struct StateSetHash {
        // State Set 的哈希函数，使用 Boost 风格的 hash_combine 算法。
        // state 具体表现为正序排列且无重复元素的 std::vector<size_t>
        size_t operator()(const std::vector<size_t> &states) const {
            size_t h = 0;
            for (size_t s : states) {
                h ^= std::hash<size_t>{}(s) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    };

}


DFA nfa_to_dfa(const NFA &nfa, bool minimize){
    // 将 NFA 转换为 DFA, 并可选择是否进行最小化
    // 参考 Subset construction

    DFA dfa;
    // 记录已经发现过的状态集和dfa状态的映射表，包括待处理的(pending_state_sets) 和已经处理过的
    std::unordered_map<std::vector<size_t>, size_t, StateSetHash> discovered_tab;

        // 判断是否属于最终状态
    const std::vector<size_t> &nfa_final_state_set = nfa.final_state_set_;
    auto is_final = [&nfa_final_state_set](const std::vector<size_t> &state_set) -> bool {
        auto it = nfa_final_state_set.begin(), it2 = state_set.begin();
        while (it != nfa_final_state_set.end() && it2 != state_set.end()){
            if (*it > *it2) ++it2;
            else if (*it < *it2) ++it;
            else return true;
        }
        return false;
    };

    // 待处理的队列
    std::queue<std::vector<size_t>> pending;
    pending.emplace(nfa.epsilon_closure({0}));
    // dfa 初始状态为 1
    discovered_tab.emplace(pending.front(), 1);
    if (is_final(pending.front())) dfa.add_final_state(1);

    while (!pending.empty()){
        std::vector<size_t> &processing_state_set = pending.front();
        size_t dfa_state = discovered_tab[processing_state_set];

        // 收集 processing_state_set 包含的所有 NFATransition
        std::vector<const NFATransition*> processing_nfa_transitions; 
        for (auto state: processing_state_set){
            for (const NFATransition &t : nfa.transition_table_[state]){
                processing_nfa_transitions.push_back(&t);
            }
        }

        if (processing_nfa_transitions.empty()){
            pending.pop();
            continue;
        }

        // 子集的边转换成 dfa 的边
        SubsetIntervalsCursor cursor = SubsetIntervalsCursor(processing_nfa_transitions);
        while (cursor.more()){
            auto [start, end, target_state_set] = cursor.next();
            target_state_set = nfa.epsilon_closure(target_state_set);
            auto it = discovered_tab.find(target_state_set);
            if (it == discovered_tab.end()){
                size_t dfa_target = dfa.new_state();
                // 判断 dfa_target 是否是最终状态
                if (is_final(target_state_set)) dfa.add_final_state(dfa_target);
                pending.push(target_state_set);
                discovered_tab.emplace(target_state_set, dfa_target);
                dfa.transition_table_[dfa_state].intervals_.emplace_back(DFAInterval{start, end, dfa_target});
            }
            else{
                size_t dfa_target = it->second;
                dfa.transition_table_[dfa_state].intervals_.emplace_back(DFAInterval{start, end, dfa_target});
            }
        }
        pending.pop();
    }
    return dfa;
}