#ifndef INCLUDED_REDUCER_HPP
#define INCLUDED_REDUCER_HPP
#include "lambda.hpp"
#include "lexer.hpp"
#include <memory>
#include <vector>

Expression reduce(std::vector<lex_unit> lex_units)
{
    enum class token
    {
        exp,
        var,
        app,
        abst
    };

    std::vector<token> sig;
    std::vector<Expression> ent;
    size_t index = 0;
    bool reducing = false;
    while (index < lex_units.size())
    {
        auto &lex = lex_units.at(index);
        if (lex.type == term::variable || lex.type == term::arg_variable)
        {
            ent.emplace_back(lex.str);
        }
        if (lex.type == term::abst_begin)
        {
            sig.emplace_back(token::abst);
        }
        if (lex.type == term::paren_begin)
        {
            sig.emplace_back(token::exp);
        }
        if (lex.type == term::paren_end || index == (lex_units.size() - 1))
        {
            bool paren = (lex.type == term::paren_end);
            while (!sig.empty() && (!paren || sig.back() != token::exp))
            {
                if (sig.back() == token::abst)
                {
                    auto arg2 = ent.back();
                    ent.pop_back();
                    auto var = ent.back().str();
                    ent.pop_back();
                    ent.emplace_back(var, arg2);
                }
                sig.pop_back();
            }
            if (ent.size() == 2)
            {
                auto arg2 = ent.back();
                ent.pop_back();
                auto arg1 = ent.back();
                ent.pop_back();
                ent.emplace_back(arg1, arg2);
            }
        }
        index++;
    }
    return ent.front();
}

#endif