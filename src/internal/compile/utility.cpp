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



