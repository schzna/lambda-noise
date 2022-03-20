#include <cctype>
#include <exception>
#include <iostream>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <string_view>

const bool debugprint = false;

namespace impl
{

    class Named
    {
    public:
        const std::string name;

        Named(std::string_view str) : name(str) {}

        bool operator==(const Named &named) const
        {
            return name == named.name;
        }

        bool operator!=(const Named &named) const
        {
            return name != named.name;
        }
    };

    struct BaseConstructor
    {
    };

    class Expression;
    class Variable;
    class Abstraction;
    class Application;

    class Expression
    {
        friend class Variable;
        friend class Abstraction;
        friend class Application;

    private:
        std::shared_ptr<Expression> rep;

        virtual Expression beta_impl(const Expression &exp) const;

    protected:
        Expression(BaseConstructor);

    public:
        Expression(std::string_view v);
        Expression(std::string_view x, const Expression &exp);
        Expression(const Expression &exp1, const Expression &exp2);

        virtual std::string str() const;
        virtual std::set<std::string> free_variable() const;

        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;
    };

    class Variable : public Expression, public Named
    {
        friend class Expression;
        friend class Abstraction;
        friend class Application;

        Variable(std::string_view v);

        virtual Expression beta_impl(const Expression &exp) const;

        virtual std::string str() const;
        virtual std::set<std::string> free_variable() const;

        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;
    };

    class Abstraction : public Expression
    {
        friend class Expression;
        friend class Variable;
        friend class Application;

        Variable arg;
        Expression exp;

        Abstraction(std::string_view x, const Expression &exp);
        Abstraction(const Variable &x, const Expression &exp);

        virtual Expression beta_impl(const Expression &exp) const;

        virtual std::string str() const;
        virtual std::set<std::string> free_variable() const;

        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;
    };

    class Application : public Expression
    {
        friend class Expression;
        friend class Variable;
        friend class Abstraction;

        Expression exp1, exp2;

        Application(const Expression &exp1, const Expression &exp2);

        virtual Expression beta_impl(const Expression &exp) const;

        virtual std::string str() const;
        virtual std::set<std::string> free_variable() const;

        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;
    };

    Expression::Expression(std::string_view v)
    {
        rep = std::shared_ptr<Variable>(new Variable(v));
    }

    Expression::Expression(std::string_view x, const Expression &exp)
    {
        rep = std::shared_ptr<Abstraction>(new Abstraction(x, exp));
    }

    Expression::Expression(const Expression &exp1, const Expression &exp2)
    {
        rep = std::shared_ptr<Application>(new Application(exp1, exp2));
    }

    Expression::Expression(BaseConstructor)
    {
        rep = nullptr;
    }

    Expression Expression::beta_impl(const Expression &exp) const
    {
        return rep->beta_impl(exp);
    }

    std::string Expression::str() const
    {
        return rep->str();
    }

    std::set<std::string> Expression::free_variable() const
    {
        return rep->free_variable();
    }

    Expression Expression::substitute(std::string_view v, const Expression &exp) const
    {
        return rep->substitute(v, exp);
    }

    Expression Expression::beta_reduction() const
    {
        return rep->beta_reduction();
    }

    Variable::Variable(std::string_view v) : Expression(BaseConstructor()), Named(v)
    {
    }

    Expression Variable::beta_impl(const Expression &exp) const
    {
        return Expression(name);
    }

    std::string Variable::str() const
    {
        return name;
    }

    std::set<std::string> Variable::free_variable() const
    {
        return {name};
    }

    Expression Variable::substitute(std::string_view v, const Expression &exp) const
    {
        if (debugprint)
            std::printf("variable(%s)::substitute(%s, %s)\n", name.c_str(), v.data(), exp.str().c_str());
        if (name == v)
        {
            return exp;
        }
        return Expression(name);
    }

    Expression Variable::beta_reduction() const
    {
        return Expression(name);
    }

    Abstraction::Abstraction(std::string_view x, const Expression &exp) : Expression(BaseConstructor()), arg(x), exp(exp)
    {
    }

    Abstraction::Abstraction(const Variable &x, const Expression &exp) : Expression(BaseConstructor()), arg(x), exp(exp)
    {
    }

    Expression Abstraction::beta_impl(const Expression &exp) const
    {
        return this->exp.substitute(arg.name, exp);
    }

    std::string Abstraction::str() const
    {
        return "λ" + arg.name + "." + exp.str();
    }

    std::set<std::string> Abstraction::free_variable() const
    {
        auto res = exp.free_variable();
        res.erase(arg.name);
        return res;
    }

    Expression Abstraction::substitute(std::string_view v, const Expression &exp) const
    {
        if (debugprint)
            std::printf("abstraction(%s, %s)::substitute(%s, %s)\n", arg.name.c_str(), this->exp.str().c_str(), v.data(), exp.str().c_str());
        if (arg.name != v)
        {
            return Expression(arg.name, this->exp.substitute(v, exp));
        }
        return Expression(arg.name, this->exp);
    }

    Expression Abstraction::beta_reduction() const
    {
        return Expression(arg.name, exp);
    }

    Application::Application(const Expression &exp1, const Expression &exp2) : Expression(BaseConstructor()), exp1(exp1), exp2(exp2)
    {
    }

    Expression Application::beta_impl(const Expression &exp) const
    {
        return Expression(exp1, exp2);
    }

    std::string Application::str() const
    {
        return "(" + exp1.str() + ")(" + exp2.str() + ")";
    }

    std::set<std::string> Application::free_variable() const
    {
        auto res = exp1.free_variable();
        res.merge(exp2.free_variable());
        return res;
    }

    Expression Application::substitute(std::string_view v, const Expression &exp) const
    {
        if (debugprint)
        {
            std::printf("application(%s, %s)::substitute(%s, %s)\n", exp1.str().c_str(), exp2.str().c_str(), v.data(), exp.str().c_str());
        }

        return Expression(exp1.substitute(v, exp), exp2.substitute(v, exp));
    }

    Expression Application::beta_reduction() const
    {
        return exp1.beta_impl(exp2);
    }

    class LambdaException : public std::exception
    {
    public:
        LambdaException() noexcept {}
        LambdaException(const LambdaException &) noexcept {}
        ~LambdaException() {}

        exception &operator=(const exception &) noexcept
        {
            return *this;
        }
        virtual const char *what() const noexcept
        {
            return "syntax error\n";
        }
    };
}

using Expression = impl::Expression;

Expression Variable(std::string_view v)
{
    return Expression(v);
}

Expression Abstraction(std::string_view x, const Expression &exp)
{
    return Expression(x, exp);
}

Expression Abstraction(const impl::Variable &x, const Expression &exp)
{
    return Expression(x, exp);
}

Expression Application(const Expression &exp1, const Expression &exp2)
{
    return Expression(exp1, exp2);
}

namespace parser
{
    enum class token
    {
        abst_begin,
        paren_begin,
        expression
    };

    Expression parse(const std::string &str)
    {
        size_t i = 0;
        std::stack<Expression> entity_stack = {};
        std::stack<token> token_stack = {};
        while (i < str.size() || entity_stack.size() > 1)
        {
            if (i < str.size())
            {
                if (str.at(i) == '\\')
                {
                    token_stack.push(token::abst_begin);
                    i++;
                    entity_stack.push(Variable(str.substr(i, 1)));
                    i += 2;
                }
                if (str.at(i) == '(')
                {
                    token_stack.push(token::paren_begin);
                    i++;
                }
                if (std::isalpha(str.at(i)))
                {
                    entity_stack.push(Variable(str.substr(i, 1)));
                    token_stack.push(token::expression);
                    i++;
                }
            }

            if (i == str.size() || str.at(i) == ')')
            {
                auto t = entity_stack.top();
                entity_stack.pop();
                token_stack.pop();
                if (token_stack.empty())
                {
                    entity_stack.push(t);
                    token_stack.push(token::expression);
                }
                if (token_stack.top() == token::abst_begin)
                {
                    auto abst = Abstraction(entity_stack.top().str(), t);
                    entity_stack.pop();
                    entity_stack.push(abst);
                }
                else if (token_stack.top() == token::paren_begin)
                {
                    entity_stack.push(t);
                    i++;
                }
                token_stack.pop();
                token_stack.push(token::expression);
                if (token_stack.size() == 1)
                {
                    i++;
                }
            }

            if (token_stack.top() == token::expression)
            {
                token_stack.pop();
                if (token_stack.empty())
                {
                }
                else if (token_stack.top() == token::expression)
                {
                    token_stack.pop();
                    auto e2 = entity_stack.top();
                    entity_stack.pop();
                    auto e = Application(entity_stack.top(), e2);
                    entity_stack.pop();
                    entity_stack.push(e);
                }
                token_stack.push(token::expression);
            }

            if (i < str.size() && str.at(i) == '\n')
            {
                i++;
            }
        }
        return entity_stack.top();
    }

}

int main()
{
    std::string str = "";
    while (1)
    {
        std::cout << "λ>";
        std::cin >> str;
        auto l = parser::parse(str);
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