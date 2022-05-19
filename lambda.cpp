#include "lambda.hpp"
#include "reducer.hpp"
#include <fstream>
#include <iostream>
#include <variant>

std::variant<Expression, Definition> parseandreduce(std::string_view str, Environment &env)
{
    auto res = reduce(lexer(str));
    auto l = res.second;
    bool is_def = (res.first.name != "");

    if (debugprint)
    {
        if (is_def)
            std::cout << res.first.str() << "\n";
        else
            std::cout << l.str() << "\n";
        for (auto &&e : l.bound_variables())
        {
            std::cout << e << " ";
        }
        std::cout << std::endl;
    }

    for (auto &&def : env)
    {
        l = l.substitute(def.name, def.exp);
    }

    auto tmp = l.beta_reduction();
    while (l.str() != tmp.str())
    {
        l = tmp;
        tmp = tmp.beta_reduction();
    }
    l = tmp;

    if (is_def)
    {
        auto entity = res.first.exp;
        for (auto &&def : env)
        {
            entity = entity.substitute(def.name, def.exp);
        }
        auto tmp = entity.beta_reduction();
        while (entity.str() != tmp.str())
        {
            entity = tmp;
            tmp = tmp.beta_reduction();
        }
        entity = tmp;
        Definition def{res.first.name, entity};
        env.insert(def);
        return def;
    }
    return l;
}

int main(int argc, char **argv)
{
    Environment env = {};
    std::string str = "";
    if (argc == 2)
    {
        std::ifstream ifs(argv[1]);
        if (!ifs)
        {
            std::cout << "not found: " << argv[1] << std::endl;
            return 1;
        }
        else
        {
            size_t line = 1;
            while (1)
            {
                std::cout << "line " << line << ": ";
                std::getline(ifs, str);
                if (str.size() > 0 && str.at(0) == '#')
                {
                    std::cout << str << "\n";
                }
                else
                {
                    std::visit([](const auto &x)
                               { std::cout << x.str() << std::endl; },
                               parseandreduce(str, env));
                }
                if (ifs.bad() || ifs.eof())
                    break;
                line++;
            }
            return 0;
        }
    }
    while (1)
    {
        std::cout << "λ>";
        std::getline(std::cin, str);
        std::visit([](const auto &x)
                   { std::cout << x.str() << std::endl; },
                   parseandreduce(str, env));
    }
    return 0;
}