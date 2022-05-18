#include "lambda.hpp"
#include "reducer.hpp"
#include <fstream>
#include <iostream>

Expression parseandreduce(std::string_view str)
{
    auto l = reduce(lexer(str));
    if (debugprint)
        std::cout << l.str() << "\n";
    auto tmp = l.beta_reduction();
    while (l.str() != tmp.str())
    {
        l = tmp;
        tmp = tmp.beta_reduction();
    }
    l = tmp;
    return l;
}

int main(int argc, char **argv)
{
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
                    std::cout << parseandreduce(str).str() << std::endl;
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
        std::cin >> str;
        std::cout << parseandreduce(str).str() << std::endl;
    }
    return 0;
}