#include <vector>

#include <gtest/gtest.h>

#include "state_set_ops.hpp"


TEST(SplitByTest, base_test){
    {
        std::vector<size_t> first = {1, 2, 3, 4, 5};
        std::vector<size_t> second = {2, 4, 6, 8};
        auto [diff, intersection] = split_by(first, second);
        ASSERT_EQ(diff, std::vector<size_t>({1, 3, 5}));
        ASSERT_EQ(intersection, std::vector<size_t>({2, 4}));
    }

    {
        std::vector<size_t> first = {1, 2, 3, 4, 5};
        std::vector<size_t> second = {6, 8};
        auto [diff, intersection] = split_by(first, second);
        ASSERT_EQ(diff, std::vector<size_t>({1, 2, 3, 4, 5}));
        ASSERT_EQ(intersection, std::vector<size_t>({}));
    }

    {
        std::vector<size_t> first = {1, 2, 3, 4, 5, 8, 9, 10};
        std::vector<size_t> second = {2, 4, 6, 8};
        auto [diff, intersection] = split_by(first, second);
        ASSERT_EQ(diff, std::vector<size_t>({1, 3, 5, 9, 10}));
        ASSERT_EQ(intersection, std::vector<size_t>({2, 4, 8}));
    }
}
