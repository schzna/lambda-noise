#include "lambda.hpp"
#include "reducer.hpp"
#include <fstream>
#include <iostream>

Expression parseandreduce(std::string_view str)
{
    auto l = reduce(lexer(str));
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
            std::cout << parseandreduce(str).str() << std::endl;
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