#ifndef INCLUDED_REDUCER_HPP
#define INCLUDED_REDUCER_HPP
#include "lambda.hpp"
#include "lexer.hpp"
#include <memory>
#include <stack>
#include <string>
#include <utility>

struct Definition
{
    std::string name;
    Expression exp;

    Definition() : name(""), exp(Constant("")) {}
    Definition(std::string_view name, const Expression &exp) : name(name), exp(exp) {}

    std::string str() const
    {
        return name + " := " + exp.str();
    }

    bool operator<(const Definition &d) const
    {
        return name < d.name;
    }
};

using Environment = std::set<Definition>;

std::pair<Definition, Expression> reduce(std::vector<lex_unit> lex_units)
{
    enum class tokenkind
    {
        exp,
        arg,
        var,
        app,
        abst,
        begin,
        constant,
        defeq
    };

    struct token
    {
        tokenkind type;
        int ref;
        token() {}
        token(tokenkind type, int ref) : type(type), ref(ref) {}
    };

    std::stack<token> sig;
    std::stack<Expression> ent, apps;
    Definition def{"", Constant("none")};
    size_t index = 0;
    while (index < lex_units.size())
    {
        auto &lex = lex_units.at(index);
        if (lex.type == term::variable)
        {
            ent.emplace(lex.str);
            sig.emplace(tokenkind::var, ent.size() - 1);
        }
        if (lex.type == term::id)
        {
            ent.emplace(lex.str, true);
            sig.emplace(tokenkind::constant, ent.size() - 1);
        }
        if (lex.type == term::defeq)
        {
            sig.emplace(tokenkind::defeq, ent.size());
            lex_units.insert(lex_units.begin() + index + 1, lex_unit("(", term::paren_begin));
            lex_units.emplace_back(")", term::paren_end);
        }
        if (lex.type == term::arg_variable)
        {
            ent.emplace(lex.str);
            sig.emplace(tokenkind::arg, ent.size() - 1);
        }
        if (lex.type == term::abst_begin)
        {
            sig.emplace(tokenkind::abst, ent.size());
        }
        if (lex.type == term::paren_begin)
        {
            sig.emplace(tokenkind::begin, ent.size());
        }
        if (lex.type == term::paren_end)
        {
            bool paren = (lex.type == term::paren_end);

            while (!sig.empty() && (!paren || sig.top().type != tokenkind::begin))
            {
                if (sig.top().type == tokenkind::exp)
                {
                    auto exp = ent.top();
                    ent.pop();
                    apps.push(exp);
                }
                if (sig.top().type == tokenkind::var || sig.top().type == tokenkind::constant)
                {
                    auto exp = ent.top();
                    ent.pop();
                    apps.push(exp);
                }
                if (sig.top().type == tokenkind::arg)
                {
                    if (apps.size() != 0)
                    {
                        auto exp = apps.top();
                        apps.pop();
                        while (!apps.empty())
                        {
                            exp = Expression(exp, apps.top());
                            apps.pop();
                        }
                        ent.push(exp);
                    }
                }
                if (sig.top().type == tokenkind::abst)
                {
                    auto exp = ent.top();
                    ent.pop();
                    auto arg = ent.top();
                    ent.pop();
                    sig.pop();
                    sig.emplace(tokenkind::exp, ent.size());
                    sig.emplace(tokenkind::exp, ent.size());
                    ent.emplace(arg.str(), exp);
                };

                sig.pop();
            }
            while (ent.size() > sig.top().ref)
            {
                apps.push(ent.top());
                ent.pop();
            }
            auto exp = apps.top();
            apps.pop();
            while (!apps.empty())
            {
                exp = Expression(exp, apps.top());
                apps.pop();
            }
            ent.push(exp);
            sig.pop();
            sig.emplace(tokenkind::exp, ent.size() - 1);
        }

        if (debugprint)
        {
            std::stack<token> tmp;
            while (!sig.empty())
            {
                tmp.push(sig.top());
                sig.pop();
            }
            while (!tmp.empty())
            {
                sig.push(tmp.top());
                std::cout << "type: ";
                switch (tmp.top().type)
                {
                case tokenkind::exp:
                    std::cout << "exp";
                    break;
                case tokenkind::arg:
                    std::cout << "arg";
                    break;
                case tokenkind::var:
                    std::cout << "var";
                    break;
                case tokenkind::app:
                    std::cout << "app";
                    break;
                case tokenkind::abst:
                    std::cout << "abst";
                    break;
                case tokenkind::begin:
                    std::cout << "begin";
                    break;
                case tokenkind::constant:
                    std::cout << "constant";
                    break;
                case tokenkind::defeq:
                    std::cout << "defeq";
                    break;
                default:
                    break;
                }
                std::cout << ", ref: " << tmp.top().ref << std::endl;
                tmp.pop();
            }
            std::cout << std::endl;
        }
        if (debugprint)
        {
            std::stack<Expression> tmp;
            while (!ent.empty())
            {
                tmp.push(ent.top());
                ent.pop();
            }
            while (!tmp.empty())
            {
                ent.push(tmp.top());
                std::cout << tmp.top().str() << std::endl;
                tmp.pop();
            }
            std::cout << std::endl
                      << std::endl;
        }

        index++;
    }
    while (!ent.empty() && (sig.top().type != tokenkind::arg) && (sig.top().type != tokenkind::defeq))
    {
        apps.push(ent.top());
        ent.pop();
        sig.pop();
    }
    auto exp = apps.top();
    apps.pop();
    while (!apps.empty())
    {
        exp = Expression(exp, apps.top());
        apps.pop();
    }

    if (!sig.empty())
    {
        if (sig.top().type == tokenkind::arg)
        {
            auto arg = ent.top();
            ent.pop();
            ent.emplace(arg.str(), exp);
        }
        if (sig.top().type == tokenkind::defeq)
        {
            auto id = ent.top();
            ent.pop();
            def = Definition(id.str(), exp);
        }
    }
    else
    {
        ent.push(exp);
    }

    if (def.name == "")
        return {def, ent.top()};
    else
        return {def, Constant("")};
}

#endif