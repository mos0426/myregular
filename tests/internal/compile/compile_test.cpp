#include <string>
#include <gtest/gtest.h>

#include "compile.hpp"
#include "parse.hpp"

TEST(CompileTest, Concatneation){
    {
        std::string expression = "123456789";
        std::string_view sv = expression;
        auto ast = parse(sv);
        auto nfa = compile_to_nfa(*ast.get());
        std::string a_str = "123456789";
        for (auto c: a_str) nfa.consume(c);
        ASSERT_TRUE(nfa.check());
    }

}


TEST(CompileTest, Plus){
    std::string expression = "a+";
    std::string_view sv = expression;
    auto ast = parse(sv);
    auto nfa = compile_to_nfa(*ast.get());
    std::string a_str = "aa";
    for (auto c: a_str) nfa.consume(c);
    ASSERT_TRUE(nfa.check());
    nfa.reset();
    std::string a_str2 = "";
    for (auto c: a_str2) nfa.consume(c);
    ASSERT_FALSE(nfa.check());
}


TEST(CompileTest, Star){
    std::string expression = "a*";
    std::string_view sv = expression;
    auto ast = parse(sv);
    auto nfa = compile_to_nfa(*ast.get());
    std::string a_str = "aa";
    for (auto c: a_str) nfa.consume(c);
    ASSERT_TRUE(nfa.check());
    nfa.reset();
    std::string a_str2 = "";
    for (auto c: a_str2) nfa.consume(c);
    ASSERT_TRUE(nfa.check());
    nfa.reset();
    std::string a_str3 = "aab";
    for (auto c: a_str3) nfa.consume(c);
    ASSERT_FALSE(nfa.check());
}


TEST(CompileTest, Union){
    std::string expression = "0|2|3|4|5|6|7|8|9";
    std::string_view sv = expression;
    auto ast = parse(sv);
    auto nfa = compile_to_nfa(*ast.get());
    std::string a_str = "5";
    for (auto c: a_str) nfa.consume(c);
    ASSERT_TRUE(nfa.check());
}


TEST(CompileTest, CharClass){
    {
        std::string expression = "[0123456789]";
        std::string_view sv = expression;
        auto ast = parse(sv);
        auto nfa = compile_to_nfa(*ast.get());
        std::string a_str = "0123456789";
        for (auto c: a_str) {
            nfa.consume(c);
            ASSERT_TRUE(nfa.check());
            nfa.reset();
        }
    }

    {
        std::string expression = "[a-z]";
        std::string_view sv = expression;
        auto ast = parse(sv);
        auto nfa = compile_to_nfa(*ast.get());
        std::string a_str = "h";
        for (auto c: a_str) nfa.consume(c);
        ASSERT_TRUE(nfa.check());
        nfa.reset();
        nfa.consume('-');
        ASSERT_FALSE(nfa.check());
    }

    {
        std::string expression = "[a-]";
        std::string_view sv = expression;
        auto ast = parse(sv);
        auto nfa = compile_to_nfa(*ast.get());
        nfa.consume('-');
        ASSERT_TRUE(nfa.check());
    }

    {
        std::string expression = "[^0-9a-zA-]";
        std::string_view sv = expression;
        auto ast = parse(sv);
        auto nfa = compile_to_nfa(*ast.get());
        nfa.consume('中');
        ASSERT_TRUE(nfa.check());
    }
}


TEST(CompileTest, Repetition){
    {
        std::string expression = "(123){3}";
        std::string_view sv = expression;
        auto ast = parse(sv);
        auto nfa = compile_to_nfa(*ast.get());
        std::string a_str = "123123123";
        for (auto c: a_str) nfa.consume(c); 
        ASSERT_TRUE(nfa.check());
        nfa.reset();
        a_str = "123123";
        for (auto c: a_str) nfa.consume(c);
        ASSERT_FALSE(nfa.check());
    }


}


TEST(CompileTest, Escape){
    {
        std::string expression = R"([+\-]?[0-9]+(\.[0-9]+)?-\w-\W-\a-\s-\S-\d-\D-\l-\u-)";
        std::string_view sv = expression;
        auto ast = parse(sv);
        auto nfa = compile_to_nfa(*ast.get());
        std::string a_str = R"(3.1415926-a-\-d- -a-1-a-a-A-)";
        for (auto c: a_str){
            nfa.consume(c);
        }
        ASSERT_TRUE(nfa.check());

        nfa.reset();
        a_str = R"(+3.1415926-F- -G- -$-2-f-g-G-)";
        for (auto c: a_str){
            nfa.consume(c);
        }
        ASSERT_TRUE(nfa.check());

        nfa.reset();
        a_str = R"(-10086-6-.-z- -&-3-&-c-C-)";
        for (auto c: a_str){
            nfa.consume(c);
        }
        ASSERT_TRUE(nfa.check());
    }
}


TEST(CompileTest, Basic){
    {
        std::string expression = R"((0|1|2|3|4|5|6|7|8|9)+\.(0|1|2|3|4|5|6|7|8|9)+)";
        std::string_view sv = expression;
        std::string expression2 = R"([0123456789]+\.[0123456789]+)";
        std::string_view sv2 = expression2;
        
        auto ast = parse(sv), ast2 = parse(sv2);
        auto nfa = compile_to_nfa(*ast.get()), nfa2 = compile_to_nfa(*ast2.get());
        std::string a_str = "3.1415926";
        for (auto c: a_str){
            nfa.consume(c);
            nfa2.consume(c);
        }
        ASSERT_TRUE(nfa.check());
        ASSERT_TRUE(nfa2.check());
    }
    

}

