#ifndef INCLUDED_REDUCER_HPP
#define INCLUDED_REDUCER_HPP
#include "lambda.hpp"
#include "lexer.hpp"
#include <memory>
#include <stack>

Expression reduce(std::vector<lex_unit> lex_units)
{
    enum class token
    {
        exp,
        var,
        app,
        abst
    };

    std::stack<token> sig;
    std::stack<Expression> ent;
    size_t index = 0;
    while (index < lex_units.size())
    {
        auto &lex = lex_units.at(index);
        if (lex.type == term::variable || lex.type == term::arg_variable)
        {
            ent.emplace(lex.str);
        }
        if (lex.type == term::abst_begin)
        {
            sig.push(token::abst);
        }
        if (lex.type == term::paren_begin)
        {
            sig.push(token::exp);
        }
        if (lex.type == term::paren_end || index == (lex_units.size() - 1))
        {
            bool paren = (lex.type == term::paren_end);
            while (!sig.empty() && (!paren || sig.top() != token::exp))
            {
                if (sig.top() == token::abst)
                {
                    auto arg2 = ent.top();
                    ent.pop();
                    auto var = ent.top().str();
                    ent.pop();
                    ent.emplace(var, arg2);
                }
                sig.pop();
            }
            if (ent.size() == 2)
            {
                auto arg2 = ent.top();
                ent.pop();
                auto arg1 = ent.top();
                ent.pop();
                ent.emplace(arg1, arg2);
            }
        }
        index++;
    }
    return ent.top();
}

#endif