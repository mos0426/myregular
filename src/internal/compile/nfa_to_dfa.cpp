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
#include "charset.hpp"
#include "utility.hpp"
#include "dfa.hpp"
#include "nfa.hpp"

namespace{

    struct Endpoint{
        uint32_t codepoint;
        bool is_start;
    };

    class NFATransitionCursor{
        // 管理 NFATransition 内码点区间上的端点，按正序输出端点流
    public:
        NFATransitionCursor() = delete;

        NFATransitionCursor(const NFATransition &t): intervals_(&t.char_set.get_intervals()){
            current_it_ = intervals_->begin();
            target_ = t.target_state;
            current_ = Endpoint{current_it_->start, true};
        };

        std::pair<Endpoint, size_t> next(){
            assert(more());
            if (current_.is_start){
                current_ = Endpoint{current_it_->end, false};
                ++current_it_;
            }
            return {current_, target_};
        };

        std::pair<Endpoint, size_t> current() const { return {current_, target_};};

        bool more() const {return current_it_ != intervals_->end();};

        friend bool operator<(const NFATransitionCursor &left, const NFATransitionCursor &right);
    
    private:
        const std::vector<Interval> *intervals_;
        std::vector<Interval>::const_iterator current_it_;
        size_t target_;
        Endpoint current_;
    };


    bool operator<(const NFATransitionCursor &left, const NFATransitionCursor &right){
        if (left.current_.codepoint < right.current_.codepoint) return true;
        else if (left.current_.codepoint == right.current_.codepoint){
            // 码点值相同时， 终点比起点更大
            return !left.current_.is_start && right.current_.is_start;
        }
        return false;
    }


    class NFATransitionCursorMerger{
    // NFATransitionCursor 归并器，把多个 NFATransitionCursor 统一管理，将多个 NFATransitionCursor 的端点流
    // 按码点正序归并输出。
    public:
        NFATransitionCursorMerger() = default;

        void add_cursor(NFATransitionCursor &&cursor){
            cursors_.push_back(std::move(cursor));
            shift_up(cursors_.size()-1);
        }

        std::pair<Endpoint, size_t> next(){
            assert(more());
            std::pair<Endpoint, size_t> result = cursors_[0].current();
            if (cursors_[0].more()){
                cursors_[0].next();
                shift_down(0);
            }
            else{
                cursors_[0] = std::move(cursors_.back());
                cursors_.pop_back();
                if (!cursors_.empty()) shift_down(0);
            }
            return result;
        };

        bool more(){return !cursors_.empty();}

        
    private:
        // cursors_ 为一个最小堆
        // cursors_ 的数据索引方式为完全二叉树 (Complete Binary Tree)
        std::vector<NFATransitionCursor> cursors_;

        void shift_down(size_t i){
            // 下沉 cursor[i], 直到 cursor[i] 大于它的所有字节点

            if ((cursors_.size() - 1) <= (i*2)) return ;
            // 左子节点和右子节点的索引
            size_t l = i*2 + 1;
            // 最小子节点的索引
            size_t min_children;
            if ((cursors_.size() - 1) < (i*2+1)){
                size_t r = i*2 + 2;
                min_children = cursors_[l] < cursors_[r] ? l : r;
            }
            else min_children = l;
            if (cursors_[l] < cursors_[i]){
                std::swap(cursors_[i], cursors_[min_children]);
                return shift_down(min_children);
            }
            return ;
        };

        void shift_up(size_t i){
            // 上浮 cursor[i]， 直到 cursor[i] 小于它的父节点

            if (i == 0) return ;
            // 父节点索引
            size_t p = (i - 1) / 2;
            
            if (cursors_[p] < cursors_[i]){
                std::swap(cursors_[i], cursors_[p]);
                return shift_up(p);
            }
            return ;
        }
    };


    class SubsetIntervalsCursor{
    public:
        SubsetIntervalsCursor() = delete;

        SubsetIntervalsCursor(std::vector<const NFATransition*> ps){
            assert(!ps.empty());
            for (auto p: ps){
                if (!p->char_set.empty()) merger_.add_cursor(NFATransitionCursor(*p));  
            }
            assert(merger_.more());
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
                                if (refcount_.empty() & !merger_.more()){
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
                                if (!refcount_.empty()) current_start_ = endpoint.codepoint;
                                return result;
                            }
                            return next();
                        }
                        // endpoint 为终点时，refcount 必然包含 target_state
                        assert(false);
                    }
                }
                // endpoint 为终点时，refcount 必然包含 target_state
                assert(false);
                return subset_interval_buffer_;
            }
        };

        bool more(){return has_more_;};

    private:
        NFATransitionCursorMerger merger_;
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
    std::vector<size_t> initial_state_set = {0};
    for (auto s: nfa.epsilon_closure({0})) initial_state_set.push_back(s);
    pending.emplace(std::move(initial_state_set));
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