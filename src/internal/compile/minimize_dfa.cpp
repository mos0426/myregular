#include <vector>
#include <cassert>
#include <utility>

#include "compile.hpp"
#include "dfa.hpp"
#include "dfa_transition.hpp"
#include "interval.hpp"
#include "dfa_interval_cursor.hpp"
#include "cursor_merger.hpp"

namespace{

    class ReverseTransitionBuilder{
    // ReverseTransition 的构造器, feed 接口接受端点流, 出现区间时会往 target_ 推入区间;
    // 目前未出现区间终点时， current_end_ = 0;
    public:
        ReverseTransitionBuilder() = delete;

        ReverseTransitionBuilder(SubsetIntervalSet &target):
            target_(target), current_start_(0), current_end_(0), refcount_({}){};

        void feed(DFAEndpoint &endpoint){
            if (refcount_.empty){
                assert(endpoint.is_start);
                current_start_ = endpoint.codepoint;
                refcount_.emplace_back(std::make_pair(endpoint.target_state, 1));
                return ;
            }

            if (endpoint.is_start){
                auto it = refcount_.begin();
                while (it != refcount_.end()){
                    if (it->first < endpoint.target_state) ++it;
                    // 出现区间终点信号，跳出，交给下一步处理
                    else if (it->first > endpoint.target_state) break;
                    else{ // it->first == endpoint.target_state
                        ++it->second;
                        return ;
                    }
                }

                // 存在重合区间起点的情况
                if (endpoint.codepoint == current_start_){
                    refcount_.emplace(it, std::make_pair(endpoint.target_state, 1));
                    return ;
                }
                
                // 能从上一个 while 循环跳出来都是出现了区间终点信号
                // 出现区间终点信号，推出区间，更新当前区间信息;
                if (current_end_ != 0){
                    push_interval();
                    refcount_.emplace(it, std::make_pair(endpoint.target_state, 1));
                    remove_zero_count();
                    // 插入新的端点计数前，refcount 不为空, 那么current_end_ 和 输入的 endpoint 之间还纯在一个区间
                    if (refcount_.size() > 1){
                        current_start_ = current_end_;
                        current_end_ = endpoint.codepoint;
                        push_interval();
                    }
                    current_start_ = endpoint.codepoint;
                    current_end_ = 0;
                }
                else{ // current_end_ == 0 
                    current_end_ = endpoint.codepoint;
                    push_interval();
                    refcount_.emplace(it, std::make_pair(endpoint.target_state, 1));
                    current_start_ = endpoint.codepoint;
                    current_end_ = 0;
                }
                return ;

            }
            else{ // !endpoint.is_start 
                auto it = refcount_.begin();
                if (current_end_ != 0){
                    // 出现下一个区间的终点，把目前区间推出，更新当前区间信息
                    if (current_end_ != endpoint.codepoint){
                        push_interval();
                        current_start_ = current_end_;
                        current_end_ = 0;
                    }
                }
                while (it != refcount_.begin()){
                    if (it->first < endpoint.target_state) ++it;
                    else if (it->first > endpoint.target_state){
                        // 理论上不会出现一个终点类型的端点在 refcount_ 上没有计数
                        assert(false);
                    }
                    else{
                        --it->second;
                        if (it->second == 0){
                            if (current_end_ == 0) current_end_ = endpoint.codepoint;
                        }
                        return;
                        }
                    ++it;
                    }
                assert(false); // endpoint.codepoint 不可能在 refcount_ 中没有计数 
                }
                
            }

        // 检查 refcount 中的所有计数是否清零 (端点流输入完毕, refcount_ 所有计数都应该清零);
        // 推出最后一个区间.
        void finish() const {
            assert(current_end_ != 0);
            push_interval();
            assert(!remove_zero_count());
            assert(refcount_.empty());
        };

    private:
        // 被构建的目标
        SubsetIntervalSet &target_;
        uint32_t current_start_;
        uint32_t current_end_;
        // 用于记录输入的端点流中, target state 的出现次数;
        // 在 refcount 的所有操作过程中, 保持按照 target state 的值的正序排列
        std::vector<std::pair<size_t, int>> refcount_;

        // 按照目前的 current_start_, current_end_ 和 refcount_ 生成 SubsetInterval 推到 target_ 末端;
        void push_interval(){
            std::vector<size_t> target_state_set;
            for (auto it = refcount_.begin(); it != refcount_.end(); ++it){
                target_state_set.push_back(it->first);
            }
            target_.emplace_back(SubsetInterval{current_start_, current_end_, std::move(target_state_set)});
        }
        
        // 清除 refcount_ 内计数为 0 的元素
        void remove_zero_count(){
            auto it = refcount_.begin();
            while (it != refcount_.end()){
                if (it->second == 0){
                    // 记录需要移除的元素的迭代器
                    auto remove_it = it;
                    ++it;
                    while (it != refcount_.end()){
                        if (it->second != 0){
                            *remove_it = std::move(*it);
                            ++remove_it;
                        }
                        ++it;
                    }
                    refcount_.erase(remove_it, refcount_.end());
                    break;
                }
                ++it;
            }
        }
        
    }


    // 构建反向转移表 (reverse transition table)
    // 反向转移表结构为 std::vector<SubsetInterval>
    // 具体表现为一个目标状态由码点区间可映射多个起始状态 
    std::vector<SubsetIntervalSet> build_reverse_transition_table(
        std::vector<DFAIntervalSet> &transition_table
    ){

        std::vector<DFAIntervalCursor> cursors;
        // 构建 cursor 列表
        // 0 为 "死" 状态, 索引从 1 开始 
        // cursor 的 label 为 i, 即为 dfa 的起始状态
        for (int i = 1; i < transition_table.size(); ++i){
            cursors.emplace_back(DFAIntervalCursor(transition_table[i], i));
        }

        DFAIntervalCursorMerger merger = DFAIntervalCursorMerger(cursors);
        std::vector<SubsetIntervalSet> ans(transition_table.size());
        std::vector<ReverseTransitionBuilder> builders;
        for (size_t i = 0; i < ans.size(); ++i){
            builders.emplace_back(ReverseTransitionBuilder(transition_table[i]));
        }

        while (merger.more()){
            auto [endpoint, label] = merger.next();
            builders[label].feed(std::move(endpoint));
        }

        for (int i=0; i < builders.size(); ++i) builders[i].finish();
        
        return ans;
    }
}
