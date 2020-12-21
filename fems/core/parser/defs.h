#ifndef DEFS_H
#define DEFS_H

#include <string>

namespace Parser
{
    enum class ValueType { Scalar, Vector, Matrix };
    enum class Token { Indefined, Sin, Cos, Tan, Exp, Asin, Acos, Atan, Atan2, Sinh, Cosh, Tanh, Sqrt,
                       Abs, Plus, Minus, Div, Mul, Pow, Eq, Ne, Lt, Le, Gt, Ge, Not, And, Or, Constant,
                       Result, Function, Functional, Argument, Diff, Integral, Number, Variable, Variation };
    enum class TokenType { Indefined, Delimiter, Number, Function, Variable, Operator, String, Finished };
    struct idToken
    {
        std::string name;
        Token op;
    };
}
#endif // DEFS_H
