#include <variant>
#include <memory>
#include <string>
#include <vector>


class AST;


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
};


using ASTNode = std::variant<
    PlusNode,
    ConcatenationNode, 
    StarNode, 
    RepetitionNode, 
    CharNode, 
    CharClassNode
>;


class AST{
public:
    std::unique_ptr<ASTNode> data;

    template<typename T>
    AST(T &&data): data(data){};
};


AST parse(std::string_view &expression);