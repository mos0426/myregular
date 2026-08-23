#pragma once

#include <variant>
#include <memory>
#include <string>
#include <vector>


class AST;

std::unique_ptr<AST> parse(std::string_view &expression);

struct UnionNode{
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;
};

struct ConcatenationNode{
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;
};

struct PlusNode{
    std::unique_ptr<AST> left;
};

struct StarNode{
    std::unique_ptr<AST> left;
};

struct RepetitionNode{
    std::unique_ptr<AST> left;
    unsigned int min;
    unsigned int max;
};

struct CharNode{
    uint32_t codepoint;
};

struct CharRange{
    // codepoint 范围 [start, end)
    uint32_t start;
    uint32_t end;
};

struct CharClassNode{
    std::vector<CharRange> ranges;
    bool negated = false; // 是否为取反的字符类
};


using ASTNode = std::variant<
    UnionNode,
    PlusNode,
    ConcatenationNode, 
    StarNode, 
    RepetitionNode, 
    CharNode, 
    CharClassNode
>;


class AST{
public:
    ASTNode data;

    template<typename T>
    AST(T &&data): data(std::forward<T>(data)){};
};

