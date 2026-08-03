#include <string>
#include <gtest/gtest.h>

#include "parse.hpp"

TEST(ExpressionParseTest, BasicParse) {
    std::string expression1 = "(1|2|3|4|56)+[abc]+";
    std::string_view sv1 = expression1;
    auto ast1 = parse(sv1);
}