#ifndef INCLUDED_LEXER_HPP
#define INCLUDED_LEXER_HPP

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
    arg_variable,
    defeq,
    id
};

struct lex_unit
{
    std::string str;
    term type;

    lex_unit() {}
    lex_unit(std::string_view str, term type) : str(str), type(type) {}
};

std::vector<lex_unit> lexer(std::string_view str)
{
    bool is_arg = false;
    bool read_id = false;
    std::vector<lex_unit> res = {};
    std::string arg = "";
    std::string id = "";
    for (auto &&c : str)
    {
        if (std::islower(c) || std::isdigit(c))
        {
            arg = "";
            arg += c;
            if (is_arg)
            {
                res.emplace_back(arg, term::arg_variable);
            }
            else
            {
                read_id = true;
                id += c;
            }
        }
        else if (read_id)
        {
            if (id.size() == 1)
            {
                if (std::isdigit(id.at(0)))
                    res.emplace_back(id, term::id);
                else
                    res.emplace_back(id, term::variable);
            }
            else
            {
                res.emplace_back(id, term::id);
            }
            read_id = false;
            id = "";
        }
        if (c == '(')
        {
            res.emplace_back("(", term::paren_begin);
        }
        if (c == '=')
        {
            res.emplace_back("=", term::defeq);
        }
        if (c == ')')
        {
            res.emplace_back(")", term::paren_end);
        }
        if (c == '\\')
        {
            res.emplace_back("\\", term::abst_begin);
            is_arg = true;
        }
        if (c == '.')
        {
            is_arg = false;
        }
    }
    if (id.size() == 1)
    {
        if (std::isdigit(id.at(0)))
            res.emplace_back(id, term::id);
        else
            res.emplace_back(id, term::variable);
    }
    else
    {
        res.emplace_back(id, term::id);
    }
    read_id = false;
    id = "";
    return res;
}

#endif