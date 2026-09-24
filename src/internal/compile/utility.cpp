#include <vector>


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



