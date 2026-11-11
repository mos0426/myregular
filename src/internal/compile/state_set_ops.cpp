#include <vector>

#include "state_set_ops.hpp"


bool state_set_include(const std::vector<size_t> &first, const std::vector<size_t> &second){
    auto first_it = first.begin(), second_it = second.begin();
    while (first_it != first.end() && second_it != second.end()){
        if (*first_it < *second_it){
            ++first_it;
            continue;
        }
        
        if (*first_it == *second_it){
            ++second_it;
            continue;
        }

        return false;
    }

    if (second_it == second.end()) return true;
    return false;
}


std::vector<size_t> state_set_union(const std::vector<size_t> &first, const std::vector<size_t> &second){
    if (first.empty()) return second;
    if (second.empty()) return first;

    auto first_it = first.begin(), second_it = second.begin();
    std::vector<size_t> new_state_set;
    while (true){
        if (*first_it < *second_it){
            new_state_set.push_back(*first_it);
            ++first_it;
            if (first_it == first.end()){
                for (; second_it != second.end(); ++second_it) new_state_set.push_back(*second_it);
                break;
            }
        }
        else if (*first_it > *second_it){
            new_state_set.push_back(*second_it);
            ++second_it;
            if (second_it == second.end()){
                for (; first_it != first.end(); ++first_it) new_state_set.push_back(*first_it);
                break;
            }
        }
        else{ // *first == *second_it
            new_state_set.push_back(*first_it);
            ++first_it;
            if (first_it == first.end()){
                for (; second_it != second.end(); ++second_it) new_state_set.push_back(*second_it);
                break;
            }
            ++second_it;
            if (second_it == second.end()){
                for (; first_it != first.end(); ++first_it) new_state_set.push_back(*first_it);
                break;
            }
        }
    }
    return new_state_set;
}


void state_set_insert(std::vector<size_t> &state_set, size_t item){
    auto it = state_set.begin();
    while (it != state_set.end()){
        if (*it < item) ++it;
        else if (*it > item){
            state_set.insert(it, item);
            return ;
        }
        else return;
    }
    state_set.push_back(item);
    return ;
}


std::vector<size_t> state_set_diff(const std::vector<size_t> &first, const std::vector<size_t> &second){
    std::vector<size_t> result;
    auto it1 = first.begin(), it2 = second.begin();
    while (it1 != first.end() && it2 != second.end()){
        if (*it1 < *it2){
            result.push_back(*it1);
            ++it1;
        }
        else if (*it1 > *it2) ++it2;
        else ++it1, ++it2;
    }
    if (it1 != first.end()){
        while (it1 != first.end()){
            result.push_back(*it1);
            ++it1;
        }
    }
    return result;
}
