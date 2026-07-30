#include <stdexcept>
#include <string>

class ExpressionError: public std::runtime_error{
public:
    ExpressionError(
        const std::string &message,
        const std::string_view &pattern, 
        unsigned int pos, 
        const std::string &prefix="Expression Error");

    const char *what() const noexcept override{return full_message_.c_str();};
private:
    std::string message_;
    std::string pattern_;
    unsigned pos_;
    std::string full_message_;
};


class LexerError: public ExpressionError{
public:
    LexerError(
        const std::string &message,
        const std::string_view &pattern, 
        unsigned int pos):
        ExpressionError(message, pattern, pos, "Lexer Error"){};
};