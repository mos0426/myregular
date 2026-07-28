#include <variant>
#include <memory>
#include <string>


struct PlusNode
{
    uint32_t operean;
};


using ASTNode = std::variant<PlusNode>;


class AST{
public:
    std::unique_ptr<ASTNode> data;

    template<typename T>
    AST(T &&data): data(data){};
};


// AST parse(std::string &epxression);