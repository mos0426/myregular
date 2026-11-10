#include <vector>
#include <cassert>
#include <utility>
#include <numeric>

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
                    clear_zero_refcount();
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
            assert(!clear_zero_refcount());
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
        void clear_zero_refcount(){
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

    class SubsetMergerCursor{
    // 用于 dfa 最小化中求状态集的反向目标状态集;
    // next() 按照区间的正序推出反向转移目标状态集流, 不会推出区间范围, 因为在
    // dfa 最小化过程中不需要这个信息.
    public:
        SubsetMergerCursor() = delete;
        SubsetMergerCursor(SubsetIntervalCursorMerger &merger):
        merger_(merger), refcount_({}), current_end_(0), has_more_(true) 
        {
            assert(merger.more());
            buffer_ = SubsetEndpoint{0, 0, {}};
            next(); // 把 buffer_ 内填入第一个区间.
        };

        // 由 merger_ 获取正序的端点流，处理端点流并更新 refcount_, 根据 refcount_ 的更新情况判断是否出现新区间
        // 并返回新区间;
        // 出现起点端点时，端点内的目标状态集 (endpoint.target_state_set) 在 refcount_ 的内对应的计数 +1, 出现
        // refcount_ 未计数的元素插入对应的状态的一次计数;
        // 出现终点端点时, 端点内的目标状态集 (endpoint.target_state_set) 在 refcount_ 的内对应的计数 -1, 理论
        // 上来说, 此时不会存在状态在 refcount_ 上未有计数;
        std::vector<size_t> next(){
            assert(more());

            if (!merger_.more()){
                has_more_ = false;
                return buffer_;
            }

            auto [endpoint, _] = merger_.next();
            
            // 确保每次都把 merger_ 重合的端点推出来，下一个 next() 得到得端点和上一个 next() 的最末尾的
            // 端点不重合;
            if (endpoint.is_start){

                // refcount_ 为空, endpoint 是新区间的起点.
                if (refcount_.empty()){
                    current_start_ = endpoint.codepoint;
                    assert(current_end_ == 0); // 确定未出现新区间终点
                    insert_refcount(endpoint.target_state_set);
                    // 处理所有和 endpoint 重合的端点.
                    while (merger_.get_next().first.codepoint == endpoint.codepoint){
                        auto [e, _] = merger_.next();
                        insert_refcount(refresh_refcount(e.target_state_set, true));
                    }
                    // 新区间终点未出现, 继续看下一个端点.
                    return next();
                }
                // refcount 不为空, endpoint 是新区间的终点, 是下一个区间的起点.
                else{ // !refcount_.empty()
                    assert(current_end_ == 0); // 确定未出现新区间终点
                    auto insert_record = refresh_refcount(endpoint.target_state_set);

                    // 未产生新的计数, 跳过目前端点，继续看下一个端点.
                    if (insert_record.empty()){
                        return next();
                    }

                    // 产生新的计数, 出现新的区间.
                    current_end_ = endpoint.codepoint;
                    auto new_interval = push_subset_interval();
                    current_start_ = endpoint.codepoint;
                    current_end_ = 0;
                    insert_refcount(insert_record);

                    // 处理所有和 endpoint 重合的端点.
                    while (merger_.get_next().first.codepoint == endpoint.codepoint){
                        auto [e, _] = merger_.next();
                        insert_refcount(refresh_refcount(e.target_state_set, true));
                    }
                    if (merger_buffer(new_interval)){
                        return next();
                    }
                    else{
                        auto result = std::move(buffer_);
                        buffer_ = std::move(new_interval);
                        return result;
                    }
                }
            }
            else{ // !endpoint.is_start
                assert(!refcount_.empty());
                assert(current_end_ == 0);
                refresh_refcount(endpoint.target_state_set, false);
                // 处理端点类型为终点的重合端点.
                while (
                    merger_.more() &&
                    !merger_.get_next().first.is_start && 
                    merger_.get_next().first.codepoint == endpoint.codepoint
                ){
                    auto [e, _] = merger_.next();
                    refresh_refcount(e.target_state_set, false);
                }

                // 如果 refcount_ 出现计数为 0 的元素, 那就意味着出现新区间;
                // 相反则未出现新区间, 跳过当前端点, 继续看下一个端点.
                if (check_zero_refcount()){
                    current_end_ = endpoint.codepoint;
                    auto new_interval = push_subset_interval();
                    clear_zero_refcount();
                    current_start_ = endpoint.codepoint;
                    // 处理端点类型为起点的重合端点;
                    // 先前已经处理完所有端点类型为终点的重合端点, 所以后续出现的所有重合端点的类型都是起点.
                    while (
                        merger_.more() &&
                        merger_.get_next().first.codepoint == endpoint.codepoint
                    ){
                        auto [e, _] = merger_.next();
                        insert_refcount(refresh_refcount(e.target_state_set));
                    }
                    if (merger_buffer(new_interval)){
                        return next();
                    }
                    auto result = std::move(buffer_);
                    buffer_ = std::move(new_interval);
                    return result;
                }
                else{ // !check_zero_refcount()
                    return next();
                }

            }

            
        };

        bool more(){return has_more_;};

    private:
        SubsetIntervalCursorMerger &merger_;
        // refcount_ 用于反向目标状态的计数;
        // refcount_ 在所有操作过程中保持按照反向目标状态的数值大小正序排列.
        std::vector<std::pair<size_t, int>> refcount_;
        // 记录当前区间的起点;
        uint32_t current_start_;
        // 记录已经出现的可能的区间终点, 如果未出现可能的的区间终点, current_end_ = 0
        uint32_t current_end_;
        // 目前接收的端点的码点位置;
        uint32_t current_;
        // 生成的区间的缓存, next() 生成新区间时，对照缓存, 判断区间是否可合并(首尾相接, 目标状态集相等),
        // 可合并时会先合并, 然后继续找新区间，直到新区间不能和 buffer_ 合并;
        SubsetInterval buffer_;
        bool has_more_;

        std::vector<size_t> push_subset_interval(){
            std::vector<size_t> target_state_set;
            target_state_set.reserve(refcount_.size());
            for (auto c: refcount_){
                target_state_set.push_back(c.first);
            }
            return SubsetInterval(current_start_, current_end_, std::move(target_state_set));
        }
        
        // 若 buffer_ 和 other 首尾相接并且目标状态集相等，合并 buffer_ 和 other 并把合并结果更新 buffer_, 返回 true;
        // 否则, 返回 false;
        bool merger_buffer(SubsetInterval &other){
            
            // 判断是否首尾相接
            if (buffer_.end == other.start){
                // 判断目标状态集是否相等
                if (buffer_.target_state_set == other.target_state_set){
                    buffer_.end = other.end;
                    return true;
                }
            }
            return false;
        }

        // 更新 refcount, 若 is_start 为 true, 计数 +1 , 若 is_start 为 false , 计数 -1 ;
        // 返回需要插入计数的记录;
        // 返回结果记录需要插入元素和需要插入 refcount 位置的索引;
        // 需要插入的元素后续插入的位置在记录的 refcount 的插入位置的后面;
        std::vector<pair<size_t, int>> refresh_refcount(std::vector<size_t> &state_set, bool is_start){
            if (is_start){
                // i 是 state_set 的索引, j 是 refcount_ 的索引
                int i = 0, j = 0;
                // 记录需要插入元素和需要插入 refcount 位置的索引;
                // 需要插入的元素后续插入的位置在记录的 refcount 的插入位置的前面;
                std::vector<std::pair<size_t, int>> insert_record;
                while (i < state_set.size() && j < refcount_.size()){
                    if (state_set[i] < refcount_[j].first){
                        insert_record.emplace_back(std::make_pair(state_set[i], j));
                        ++i;
                    }
                    else if (state_set[i] > refcount_[j].first){
                        ++j;
                    }
                    else{ // state_set[i] == refcount_[j].first
                        ++refcount_[j];
                        ++i, ++j;
                    }
                }
                for (; i < state_set.size(); ++i){
                    insert_record.emplace_back(std::make_pair(state_set[i], refcount_.size()));
                }
                return insert_record;
            }
            else{ // !is_start
                auto it1 = state_set.begin();
                auto it2 = refcount_.begin();
                while (it1 != state_set.end()){
                    if (*it1 == it2->first){
                        --it2->second;
                        assert(it2->second >= 0);
                        ++it1, ++it2;
                    }
                    else if (*it1 > it2->first){
                        ++it2;
                    }
                    else{
                        // 正常来说不可能出现 state_set 内的元素在 refcount_ 内没有计数;
                        assert(false);
                    }
                    assert(it2 != refcount_.end());
                }
                return {};
            }
        };
        
        void insert_refcount(std::vector<pair<size_t, int>> &insert_record){
            int l_idx = refcount_.size();
            refcount_.resize(refcount_.size() + insert_record.size());
            int n = insert_record.size() - 1;
            int r_idx = refcount_.size() - 1;

            // 插入点最末尾的情况.
            while (insert_record[n] == refcount_.size()){
                refcount_[r_idx] = std::make_pair(insert_record[n].first, 1);
                --r_idx;
                --n;
                assert(n >= 0);
                assert(r_idx >= 0);
            }

            while (l_idx >= 0 && n >= 0){
                if (l_idx == insert_record[n].second){
                    refcount_[r_idx] = refcount_[l_idx];
                    --r_idx, --l_idx;
                    refcount_[r_idx] = std::make_pair(insert_record[n].first, 1);
                    --r_idx, --n;
                }
                else{ // l_idx != insert_record[n].second
                    refcount_[r_idx] = refcount_[l_idx];
                    --r_idx, --l_idx;
                }
            }
        }

        // 清除 refcount_ 内计数为 0 的元素.
        void clear_zero_refcount(){
            auto it = refcount_.begin();
            while (it != refcount_.end()){
                if (it->second != 0) ++it;
                else{
                    // 需要删除的元素的起点位置
                    auto it2 = it;
                    ++it;
                    while (it != refcount_.end()){
                        if (it->second != 0){
                            *it2 = *it;
                            ++it, ++it2;
                        }
                        else{
                            ++it;
                        }
                    }
                    refcount_.erase(it2, refcount_.end());
                }
            } 
        }

        // 检查 refcount_ 是否存在计数为 0 的元素.
        bool check_zero_refcount(){
            for (auto c: refcount_){
                if (c.second == 0) return true;
            }
            return false;
        }
    }

}
