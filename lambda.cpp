#include "lambda.hpp"
#include "reducer.hpp"
#include <iostream>

int main()
{
    std::string str = "";
    while (1)
    {
        std::cout << "λ>";
        std::cin >> str;
        auto l = reduce(lexer(str));
        auto tmp = l.beta_reduction();
        while (l.str() != tmp.str())
        {
            l = tmp;
            tmp = tmp.beta_reduction();
        }
        l = tmp;
        std::cout << l.str() << "\n";
    }
    return 0;
}