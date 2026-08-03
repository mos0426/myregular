#include <string>
#include <gtest/gtest.h>

#include "parse.hpp"

TEST(ExpressionParseTest, BasicParse) {
    std::string expression1 = "(1|2|3|4|56)+[abc]+";
    std::string_view sv1 = expression1;
    auto ast1 = parse(sv1);
    EXPECT_NE(ast1, nullptr);
    bool is_concatenation_node = std::holds_alternative<ConcatenationNode>(ast1->data);
    EXPECT_TRUE(is_concatenation_node);

    std::string expression2 = "a{1,3}b{2,4}c{5,}";
    std::string_view sv2 = expression2;
    auto ast2 = parse(sv2);
    EXPECT_NE(ast2, nullptr);
    bool is_concatenation_node2 = std::holds_alternative<ConcatenationNode>(ast2->data);
    EXPECT_TRUE(is_concatenation_node2);

    std::string expression3 = "(12|3|4|5)+.*";
    std::string_view sv3 = expression3;
    auto ast3 = parse(sv3);
    EXPECT_NE(ast3, nullptr);
    bool is_concatenation_node3 = std::holds_alternative<ConcatenationNode>(ast3->data);
    EXPECT_TRUE(is_concatenation_node3);
}