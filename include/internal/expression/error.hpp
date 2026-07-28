#include <stdexcept>
#include <string>

class LexerError: public std::runtime_error{
public:
    LexerError(const std::string &message, const std::string_view &pattern, unsigned int pos);

    const char *what() const noexcept override{return full_message_.c_str();};
private:
    std::string message_;
    std::string pattern_;
    unsigned pos_;
    std::string full_message_;
};