#ifndef INCLUDED_LAMBDA_HPP
#define INCLUDED_LAMBDA_HPP

#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

const bool debugprint = false;

namespace impl
{

    char next_letter(char c)
    {
        int i = static_cast<int>(c - 'a');
        if (i > 26)
            return c;
        return static_cast<char>((i + 1) % 26) + 'a';
    }

    class Expression;
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

    class Variable;
    class Abstraction;
    class Application;
    class Constant;

    class Expression
    {
        friend class Variable;
        friend class Abstraction;
        friend class Application;
        friend class Constant;

    private:
        std::shared_ptr<Expression> rep;

        virtual Expression beta_impl(const Expression &exp) const;

    protected:
        Expression(BaseConstructor);

    public:
        Expression(std::string_view v, bool is_constant);
        Expression(std::string_view x, const Expression &exp);
        Expression(const Expression &exp1, const Expression &exp2);

        virtual ~Expression() {}

        virtual std::string str() const;
        virtual std::set<std::string> free_variables() const;
        virtual std::set<std::string> bound_variables() const;

        virtual Expression avoid(const std::set<std::string> &v) const;
        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;
    };

    class Variable : public Expression, public Named
    {
    private:
        friend class Expression;
        friend class Abstraction;
        friend class Application;
        friend class Constant;

        Variable(std::string_view v);

        virtual Expression beta_impl(const Expression &exp) const;

        virtual std::string str() const;
        virtual std::set<std::string> free_variables() const;
        virtual std::set<std::string> bound_variables() const;

        virtual Expression avoid(const std::set<std::string> &v) const;
        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;

    public:
        virtual ~Variable() {}
    };

    class Abstraction : public Expression
    {
    private:
        friend class Expression;
        friend class Variable;
        friend class Application;
        friend class Constant;

        Variable arg;
        Expression exp;

        Abstraction(std::string_view x, const Expression &exp);
        Abstraction(const Variable &x, const Expression &exp);

        virtual Expression beta_impl(const Expression &exp) const;

        virtual std::string str() const;
        virtual std::set<std::string> free_variables() const;
        virtual std::set<std::string> bound_variables() const;

        virtual Expression avoid(const std::set<std::string> &v) const;
        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;

    public:
        virtual ~Abstraction() {}
    };

    class Application : public Expression
    {
    private:
        friend class Expression;
        friend class Variable;
        friend class Abstraction;
        friend class Constant;

        Expression exp1, exp2;

        Application(const Expression &exp1, const Expression &exp2);

        virtual Expression beta_impl(const Expression &exp) const;

        virtual std::string str() const;
        virtual std::set<std::string> free_variables() const;
        virtual std::set<std::string> bound_variables() const;

        virtual Expression avoid(const std::set<std::string> &v) const;
        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;

    public:
        virtual ~Application() {}
    };

    class Constant : public Expression, public Named
    {
    private:
        friend class Expression;
        friend class Variable;
        friend class Abstraction;
        friend class Application;

        Constant(std::string_view name);

        virtual Expression beta_impl(const Expression &exp) const;

        virtual std::string str() const;
        virtual std::set<std::string> free_variables() const;
        virtual std::set<std::string> bound_variables() const;

        virtual Expression avoid(const std::set<std::string> &v) const;
        virtual Expression substitute(std::string_view v, const Expression &exp) const;
        virtual Expression beta_reduction() const;

    public:
        virtual ~Constant() {}
    };

    Expression::Expression(std::string_view v, bool is_constant = false)
    {
        if (is_constant)
        {
            rep = std::shared_ptr<Constant>(new Constant(v));
        }
        else
        {
            rep = std::shared_ptr<Variable>(new Variable(v));
        }
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

    std::set<std::string> Expression::free_variables() const
    {
        return rep->free_variables();
    }

    std::set<std::string> Expression::bound_variables() const
    {
        return rep->bound_variables();
    }

    Expression Expression::avoid(const std::set<std::string> &v) const
    {
        return rep->avoid(v);
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
        if (debugprint)
        {
            std::printf("variable(%s)::beta_impl(%s)\n", name.c_str(), exp.str().c_str());
        }
        return Expression(Expression(name), exp);
    }

    std::string Variable::str() const
    {
        return name;
    }

    std::set<std::string> Variable::free_variables() const
    {
        return {name};
    }

    std::set<std::string> Variable::bound_variables() const
    {
        return {};
    }

    Expression Variable::avoid(const std::set<std::string> &v) const
    {
        return Expression(name);
    }

    Expression Variable::substitute(std::string_view v, const Expression &exp) const
    {
        if (debugprint)
            std::printf("variable(%s)::substitute(%s, %s)\n", name.c_str(), v.data(), exp.str().c_str());

        auto res = exp;

        if (name != v)
        {
            res = Expression(name);
        }
        if (debugprint)
            std::printf("ret %s\n", res.str().c_str());
        return res;
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
        if (debugprint)
        {
            std::printf("abstraction(%s, %s)::beta_impl(%s)\n", arg.str().c_str(), this->exp.str().c_str(), exp.str().c_str());
        }

        auto vars = exp.free_variables();
        vars.merge(exp.bound_variables());

        return this->exp.avoid(vars).substitute(arg.name, exp);
    }

    std::string Abstraction::str() const
    {
        return "(??" + arg.name + "." + exp.str() + ")";
    }

    std::set<std::string> Abstraction::free_variables() const
    {
        auto res = exp.free_variables();
        res.erase(arg.name);
        return res;
    }

    std::set<std::string> Abstraction::bound_variables() const
    {
        auto res = exp.bound_variables();
        res.insert(arg.name);
        return res;
    }

    Expression Abstraction::avoid(const std::set<std::string> &v) const
    {
        if (v.contains(arg.name))
        {
            auto used = exp.bound_variables();
            used.merge(std::set<std::string>(v));
            char tmp = next_letter(arg.name.at(0));
            while (used.contains(std::string(&tmp)))
            {
                tmp = next_letter(tmp);
            }
            std::string newname = {tmp};
            return Expression(newname, this->exp.substitute(arg.name, Expression(newname)));
        }
        return Expression(arg.name, exp);
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
        if (debugprint)
        {
            std::printf("abstraction(%s, %s)::beta_reduction()\n", arg.str().c_str(), exp.str().c_str());
        }
        return Expression(arg.name, exp.beta_reduction());
    }

    Application::Application(const Expression &exp1, const Expression &exp2) : Expression(BaseConstructor()), exp1(exp1), exp2(exp2)
    {
    }

    Expression Application::beta_impl(const Expression &exp) const
    {
        if (debugprint)
        {
            std::printf("application(%s, %s)::beta_impl(%s)\n", exp1.str().c_str(), exp2.str().c_str(), exp.str().c_str());
        }
        return Expression(Expression(exp1, exp2), exp);
    }

    std::string Application::str() const
    {
        return "(" + exp1.str() + " " + exp2.str() + ")";
    }

    std::set<std::string> Application::free_variables() const
    {
        auto res = exp1.free_variables();
        res.merge(exp2.free_variables());
        return res;
    }

    std::set<std::string> Application::bound_variables() const
    {
        auto res = exp1.bound_variables();
        res.merge(exp2.bound_variables());
        return res;
    }

    Expression Application::avoid(const std::set<std::string> &v) const
    {
        return Expression(exp1.avoid(v), exp2.avoid(v));
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
        if (debugprint)
        {
            std::printf("application(%s, %s)::beta_reduction()\n", exp1.str().c_str(), exp2.str().c_str());
        }
        return (exp1.beta_reduction()).beta_impl(exp2.beta_reduction());
    }

    Constant::Constant(std::string_view name) : Expression(BaseConstructor()), Named(name)
    {
    }

    Expression Constant::beta_impl(const Expression &exp) const
    {
        if (debugprint)
        {
            std::printf("variable(%s)::beta_impl(%s)\n", name.c_str(), exp.str().c_str());
        }
        return Expression(Expression(name), exp);
    }

    std::string Constant::str() const
    {
        return name;
    }

    std::set<std::string> Constant::free_variables() const
    {
        return {};
    }

    std::set<std::string> Constant::bound_variables() const
    {
        return {};
    }

    Expression Constant::avoid(const std::set<std::string> &v) const
    {
        return Expression(name, true);
    }

    Expression Constant::substitute(std::string_view v, const Expression &exp) const
    {
        if (debugprint)
            std::printf("constant(%s)::substitute(%s, %s)\n", name.c_str(), v.data(), exp.str().c_str());

        auto res = exp;

        if (name != v)
        {
            res = Expression(name, true);
        }
        if (debugprint)
            std::printf("ret %s\n", res.str().c_str());
        return res;
    }

    Expression Constant::beta_reduction() const
    {
        return Expression(name);
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

Expression Constant(std::string_view name)
{
    return Expression(name, true);
}

#endif