#ifndef NODE_H
#define NODE_H

#include <variant>
#include <memory>
#include "defs.h"
#include "value/value.h"

using namespace std;
using namespace Parser;


template <class T> array<double, 3> TValue<T>::x{0, 0, 0};

template <class T> class TNode
{
private:
    Token tok = Token::Indefined;
    variant<TValue<T>, TValue<T>*> val;
    shared_ptr<TNode> left;
    shared_ptr<TNode> right;
    static matrix<double> fe_coord;
public:
    TNode(void) {}
    TNode(TValue<T> v) : tok{Token::Number}, val{v} {}
    TNode(TValue<T> *v) : tok{Token::Variable}, val{v} {}
    TNode(Token t, shared_ptr<TNode> n) : tok{t}, right{n} {}
    TNode(shared_ptr<TNode> lhs,  Token t, shared_ptr<TNode> rhs) : tok{t}, left{make_shared<TNode>(*lhs)}, right{make_shared<TNode>(*rhs)} {}
    TNode(const TNode &rhs) : tok{rhs.tok}, val{rhs.val}, left{rhs.left}, right{rhs.right} {}
    ~TNode(void) noexcept {}
    TValue<T> value(void)
    {
        switch (tok)
        {
        case Token::Number:
            return get<0>(val);
        case Token::Variable:
            return *get<1>(val);
        case Token::Plus:
            if (left == nullptr) // unary
                return right->value();
            else
                return left->value() + right->value();
        case Token::Minus:
            if (left == nullptr)
                return -(right->value());
            else
                return left->value() - right->value();
        case Token::Mul:
            return left->value() * right->value();
        case Token::Div:
            return left->value() / right->value();
        case Token::Pow:
            return pow(left->value().asScalar(), right->value().asScalar());
        case Token::Eq:
            return (left->value().asScalar() == right->value().asScalar()) ? 1 : 0;
        case Token::Ne:
            return (left->value().asScalar() == right->value().asScalar()) ? 0 : 1;
        case Token::Lt:
            return (left->value().asScalar() < right->value().asScalar()) ? 1 : 0;
        case Token::Le:
            return (left->value().asScalar() <= right->value().asScalar()) ? 1 : 0;
        case Token::Gt:
            return (left->value().asScalar() > right->value().asScalar()) ? 1 : 0;
        case Token::Ge:
            return (left->value().asScalar() >= right->value().asScalar()) ? 1 : 0;
        case Token::And:
            return left->value().asScalar() and right->value().asScalar();
        case Token::Or:
            return left->value().asScalar() or right->value().asScalar();
        case Token::Not:
            return not right->value().asScalar();
        case Token::Abs:
            return fabs(right->value().asScalar());
        case Token::Sin:
            return sin(right->value().asScalar());
        case Token::Cos:
            return cos(right->value().asScalar());
        case Token::Tan:
            return tan(right->value().asScalar());
        case Token::Exp:
            return exp(right->value().asScalar());
        case Token::Asin:
            return asin(right->value().asScalar());
        case Token::Acos:
            return acos(right->value().asScalar());
        case Token::Atan:
            return atan(right->value().asScalar());
        case Token::Sinh:
            return sinh(right->value().asScalar());
        case Token::Cosh:
            return cosh(right->value().asScalar());
        case Token::Tanh:
            return tanh(right->value().asScalar());
        case Token::Sqrt:
            return sqrt(right->value().asScalar());
        case Token::Atan2:
            return atan2(left->value().asScalar(), right->value().asScalar());
        case Token::Variation:
            return var(left->value(), right->value());
        case Token::Diff:
            return diff(left->value(), right->value());
        case Token::Integral:
            return integral(right);
        default:
            break;
        }
        return TValue<T>();
    }
    TNode &operator = (const TNode &rhs)
    {
        val = rhs.val;
        tok = rhs.tok;
        left = rhs.left;
        right = rhs.right;
        return *this;
    }
    void set_fe(const matrix<double>& fec)
    {
        fe_coord = fec;
    }
    TValue<T> integral(const shared_ptr<TNode> code)
    {
        matrix<double> res(T::size() * T::freedom(), T::size() * T::freedom());

        for (auto i = 0u; i < T::w().size(); i++)
        {
            TValue<T>::x[0] = T::xi()[i];
            TValue<T>::x[1] = T::eta()[i];
            TValue<T>::x[2] = T::psi()[i];
            res += code->value().asMatrix();
        }
        return TValue<T>(res * fabs(T::jacobian(fe_coord)));
    }
};

#endif // NODE_H
