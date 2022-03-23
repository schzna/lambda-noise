#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

enum class term
{
    abst_begin,
    paren_begin,
    paren_end,
    variable,
    arg_variable
};

struct lex_unit
{
    std::string str;
    term type;
};

std::vector<lex_unit> lexer(std::string_view str)
{
    bool is_arg = false;
    std::vector<lex_unit> res = {};
    std::string tmp = "";
    for (auto &&c : str)
    {
        if (c == '(')
        {
            res.emplace_back("(", term::paren_begin);
        }
        if (c == ')')
        {
            res.emplace_back(")", term::paren_end);
        }
        if (c == '\\')
        {
            res.emplace_back("\\", term::paren_end);
            is_arg = true;
        }
        if (std::isalpha(c))
        {
            tmp = "";
            tmp += c;
            if (is_arg)
            {
                res.emplace_back(tmp, term::arg_variable);
            }
            else
            {
                res.emplace_back(tmp, term::variable);
            }
        }
        if (c == '.')
        {
            is_arg = false;
        }
    }
    return res;
}