#include <vector>

#include <gtest/gtest.h>

#include "state_set_ops.hpp"

TEST(StateSetDiffTest, base_test){
    {
        std::vector<size_t> first = {1, 2, 3, 4, 5};
        std::vector<size_t> second = {2 ,3, 5};
        std::vector<size_t> result = state_set_diff(first, second);
        ASSERT_EQ(result, std::vector<size_t>({1, 4}));
    }

    {
        std::vector<size_t> first = {1, 2, 3, 4, 5};
        std::vector<size_t> second = {6 , 7, 8};
        std::vector<size_t> result = state_set_diff(first, second);
        ASSERT_EQ(result, std::vector<size_t>({1, 2, 3, 4, 5}));
    }

    {
        std::vector<size_t> first = {1, 2, 3, 4, 5};
        std::vector<size_t> second = {2 ,3};
        std::vector<size_t> result = state_set_diff(first, second);
        ASSERT_EQ(result, std::vector<size_t>({1, 4, 5}));
    }
}