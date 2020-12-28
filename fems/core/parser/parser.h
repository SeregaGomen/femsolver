#ifndef PARSER_H
#define PARSER_H

#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <cstring>
#include <list>
#include <map>
#include "defs.h"
#include "node.h"
#include "mesh/mesh.h"
#include "error/error.h"
#include "value/value.h"

using namespace std;
using namespace Parser;

template <class T> matrix<double> TNode<T>::fe_coord{};

template <class T> class TParser
{
private:
    bool is_predicate = false;
    vector<pair<string, TValue<T>>> argument;               // Список названий аргументов искомых функций
    vector<pair<string, TValue<T>>> result;                 // Таблица результирующих функций
    vector<pair<string, TNode<T>>> constant;                // Таблица констант
    vector<pair<string, TNode<T>>> load;                    // Таблица нагрузок
    vector<pair<string, TNode<T>>> function;                // Таблица функций
    vector<pair<string, TNode<T>>> functional;              // Таблица функционалов
    list<tuple<string, int, TNode<T>, TNode<T>>> bc_list;   // Список граничных условий
    list<string> program;
    string token;
    char* expression = nullptr;
    Parser::Token tok = Token::Indefined;
    Parser::TokenType token_type = TokenType::Indefined;
    // Список операторов
    vector<idToken> operatorList{
                                    { "ARGUMENT", Token::Argument },
                                    { "CONSTANT", Token::Constant },
                                    { "RESULT", Token::Result },
                                    { "LOAD", Token::Load },
                                    { "FUNCTION", Token::Function },
                                    { "FUNCTIONAL", Token::Functional }
                                };
    // Список функций
    vector<idToken> functionList{
                                    { "SQRT", Token::Sqrt },
                                    { "SIN", Token::Sin },
                                    { "COS", Token::Cos },
                                    { "TAN", Token::Tan },
                                    { "EXP", Token::Exp },
                                    { "ASIN", Token::Asin },
                                    { "ACOS", Token::Acos },
                                    { "ATAN", Token::Atan },
                                    { "ATAN2", Token::Atan2 },
                                    { "SINH", Token::Sinh },
                                    { "COSH", Token::Cosh },
                                    { "TANH", Token::Tanh },
                                    { "DIFF", Token::Diff },
                                    { "INTEGRAL", Token::Integral },
                                    { "ABS", Token::Abs }
                                };
    // Список логических операций
    vector<idToken> booleanList{
                                    { "NOT", Token::Not },
                                    { "AND", Token::And },
                                    { "OR", Token::Or }
                                };

    // Список операций
    vector<idToken> opeartionList{
                                    { "+", Token::Plus },
                                    { "-", Token::Minus },
                                    { "*", Token::Mul },
                                    { "/", Token::Div },
                                    { "**", Token::Pow },
                                    { "==", Token::Eq },
                                    { ">", Token::Gt },
                                    { "<", Token::Lt },
                                    { ">=", Token::Ge },
                                    { "<=", Token::Le },
                                    { "<>", Token::Ne },
                                    { "var", Token::Variation }
                                };
    Parser::TokenType get_token(void);
    bool is_delim(char chr) noexcept
    {
        return (strchr(" +-*/()=.,><\t\n", chr) or chr == 0) ? true : false;
    }
    bool is_name(string) noexcept;
    bool is_find(vector<idToken> &table, string id, Token &token) noexcept
    {
        transform(id.begin(), id.end(),id.begin(), ::toupper);
        for (auto it : table)
            if (it.name == id)
            {
                token = it.op;
                return true;
            }
        return false;
    }
    template <class P> bool is_find(vector<pair<string, P>> &table, string id) noexcept
    {
        return find_if( table.begin(), table.end(), [id](pair<string, P> i) { return i.first == id; } ) == table.end() ? false : true;
    }
    template <class P> void set_value(vector<pair<string, P>> &table, string name, P value)
    {
        auto ptr = find_if( table.begin(), table.end(), [name](pair<string, P> i) { return i.first == name; } );

        if (ptr == table.end())
            set_error(Error::InternalError);
        ptr->second = value;
    }
    template <class P> void get_value(vector<pair<string, P>> &table, string name, P& value)
    {
        auto ptr = find_if( table.begin(), table.end(), [name](pair<string, P> i) { return i.first == name; } );

        if (ptr == table.end())
            set_error(Error::InternalError);
        value = ptr->second;
    }
    template <class P> int get_name_no(vector<pair<string, P>> &table, string name)
    {
        auto ptr = find_if(table.begin(), table.end(), [name](pair<string, P> i) { return i.first == name; } );

        if (ptr == table.end())
            set_error(Error::InternalError);
        return int(ptr - table.begin());
    }
    void set_error(Error error)
    {
        throw TError(error);
    }
    void put_back(void)
    {
        expression -= token.length();
    }
    void compile(void);
    void assignment(void);
    ValueType get_exp(TNode<T>&);
    ValueType token_or(TNode<T>&);
    ValueType token_and(TNode<T>&);
    ValueType token_not(TNode<T>&);
    ValueType token_add(TNode<T>&);
    ValueType token_func(TNode<T>&);
    ValueType token_mul(TNode<T>&);
    ValueType token_var(TNode<T>&);
    ValueType token_pow(TNode<T>&);
    ValueType token_un(TNode<T>&);
    ValueType token_bracket(TNode<T>&);
    ValueType token_prim(TNode<T>&);
    void get_variable(Token);
public:
    TParser(void) noexcept {}
    ~TParser(void) noexcept {}
    void set_program(const list<string>& prog)
    {
        if (not prog.size())
            throw TError(Error::EmptyProgram);
        program = prog;
        compile();
    }
    void set_data(const vector<T> &v) noexcept
    {
        for (auto &i: result)
            i.second = v;
    }
    TValue<T> run(const matrix<double>& fe)
    {
        functional.begin()->second.set_fe(fe);
        return functional.begin()->second.value();
    }
    void get_boundary_conditions(TMesh&, list<tuple<int, int, int, double>>&);
};

template <class T> void TParser<T>::compile(void)
{
    for (auto str : program)
    {
        // Удаляем пробелы в начале и конце строки
        str.erase(0, str.find_first_not_of(" \t\n\r\f\v")).erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
        if (not str.length())
            continue;
        expression = const_cast<char*>(str.c_str());
        tok = Token::Indefined;
        token_type = TokenType::Indefined;
        while (1)
        {
            if (token_type == TokenType::Finished)
                break;
            if ((token_type = get_token()) == TokenType::Variable)
            {
                put_back();
                assignment();
            }
            if (token_type == TokenType::Operator)
                get_variable(tok);
            if (token_type == TokenType::Delimiter)
                set_error((token[0] == ')') ? Error::Bracket : Error::Syntax);
        }
    }
}

template <class T> void TParser<T>::get_variable(Token cur_tok)
{
    Token tmp;
    string name;
    TNode<T> exp;
    ValueType type;

    get_token();
    if (token_type not_eq TokenType::Variable)
        set_error(Error::Syntax);
    if (not is_name(token))
        set_error(Error::Syntax);
    if (is_find(operatorList, token, tmp) or is_find(functionList, token, tmp) or is_find(booleanList, token, tmp))
        set_error(Error::InvalidIdentifier);
    if (is_find(constant, token) or is_find(argument, token) or is_find(function, token) or is_find(result, token) or is_find(functional, token))
        set_error(Error::VariableOverride);
    name = token;
    switch (cur_tok)
    {
    case Token::Argument:
        argument.push_back(pair<string, TValue<T>>(name, TValue<T>()));
        break;
    case Token::Constant:
        constant.push_back(pair<string, TNode<T>>(name, TNode<T>()));
        break;
    case Token::Result:
        result.push_back(pair<string, TValue<T>>(name, TValue<T>()));
        break;
    case Token::Load:
        load.push_back(pair<string, TValue<T>>(name, TValue<T>()));
        break;
    case Token::Function:
        function.push_back(pair<string, TNode<T>>(name, TNode<T>()));
        break;
    case Token::Functional:
        functional.push_back(pair<string, TNode<T>>(name, TNode<T>()));
        break;
    default:
        set_error(Error::Syntax);
    }

    get_token();
    if (token == "=")
    {
        if (cur_tok == Token::Argument or cur_tok == Token::Result)
            set_error(Error::InvalidInitialisation);
        type = get_exp(exp);
        if (cur_tok == Token::Constant)
        {
            if (type not_eq ValueType::Scalar)
                set_error(Error::InvalidOperation);
            set_value(constant, name, exp);
        }
        else if (cur_tok == Token::Load)
        {
            if (type not_eq ValueType::Scalar)
                set_error(Error::InvalidOperation);
            set_value(load, name, exp);
        }
        else if (cur_tok == Token::Function)
        {
            if (type not_eq ValueType::Vector)
                set_error(Error::InvalidOperation);
            set_value(function, name, exp);
        }
        else if (cur_tok == Token::Functional)
        {
            if (type not_eq ValueType::Vector)
                set_error(Error::InvalidOperation);
            set_value(functional, name, exp);
        }
    }
    if (token_type == TokenType::Finished)
        return;
    if (token not_eq ",")
        set_error(Error::Syntax);
    get_variable(cur_tok);
}

template <class T> void TParser<T>::assignment(void)
{
    int bc_type;
    string name;
    TNode<T> val,
             pred;
    ValueType type;

    get_token();
    if (not is_name(token))
        set_error(Error::InvalidIdentifier);
    name = token;
    get_token();

    if (token == "(")
    {
        is_predicate = true;
        // Граничное условие или сосредоточенная нагрузка
        if (is_find(result, name))
            bc_type = 1;
        else if (is_find(load, name))
            bc_type = 2;
        else
            set_error(Error::InvalidBoundaryCondition);
        type = get_exp(pred);
        if (type not_eq ValueType::Scalar)
            set_error(Error::Syntax);
        if (token not_eq ")")
            set_error(Error::Syntax);
        get_token();
        if (token not_eq "=")
            set_error(Error::Syntax);
        type = get_exp(val);
        if (type not_eq ValueType::Scalar)
            set_error(Error::Syntax);
        bc_list.push_back(make_tuple(name, bc_type, pred, val));
        is_predicate = false;
        return;
    }


    if (token not_eq "=")
        set_error(Error::Syntax);
    type = get_exp(val);
    if (is_find(constant, name))
    {
        if (type not_eq ValueType::Scalar)
            set_error(Error::InvalidOperation);
        set_value(constant, name, val);
    }
    else if (is_find(load, name))
    {
        if (type not_eq ValueType::Scalar)
            set_error(Error::InvalidOperation);
        set_value(load, name, val);
    }
    else if (is_find(function, name))
    {
        if (type not_eq ValueType::Vector)
            set_error(Error::InvalidOperation);
        set_value(function, name, val);
    }
    else if (is_find(functional, name))
    {
        if (type not_eq ValueType::Matrix)
            set_error(Error::InvalidOperation);
        set_value(functional, name, val);
    }
    else if (is_find(argument, name))
        set_error(Error::AssignmentArgument);
    else if (is_find(result, name))
        set_error(Error::AssignmentResult);
    else
        set_error(Error::UndefinedVariable);
}

template <class T> TokenType TParser<T>::get_token(void)
{
    token.clear();
    token_type = TokenType::Indefined;
    tok = Token::Indefined;
    if (*expression == 0)
        return (token_type = TokenType::Finished);
    while (*expression == ' ' or *expression == '\t')
        expression++;
    if (strchr(" +-*/()=.,><&", *expression))
    {
        token = *expression++;
        // Проверка на наличие двойного разделителя
        if (*expression and strchr("=><*",*expression))
            if (is_find(opeartionList, token + *expression, tok))
                token += *expression++;
        return (token_type = TokenType::Delimiter);
    }
    if (isdigit(*expression))
    {
        while (isdigit(*expression))
            token += *expression++;
        if (*expression == '.')
        {
            token += *expression++;
            while (isdigit(*expression))
                token += *expression++;
        }
        if (*expression == 'E' or *expression == 'e')
        {
            token += *expression++;
            if (*expression not_eq '+' and *expression not_eq '-' )
                set_error(Error::Syntax);
            token += *expression++;
            while (isdigit(*expression))
                token += *expression++;
        }
        return (token_type = TokenType::Number);
    }
    if (isalpha(*expression) or *expression == '_')
    {
        while (not is_delim(*expression))
            token += *expression++;
        token_type = TokenType::String;
    }
    if (token_type == TokenType::String)
    {
        if (is_find(operatorList, token, tok))
            return token_type = TokenType::Operator;
        if (is_find(functionList, token,tok))
            return token_type = TokenType::Function;
        if (is_find(booleanList, token,tok))
            return token_type = TokenType::Delimiter;
        return token_type = TokenType::Variable;
    }
    return token_type;
}

template <class T> bool TParser<T>::is_name(string token) noexcept
{
    unsigned i;

    if (not isalpha(token[0]) and token[0] not_eq '_')
        return false;
    for (i = 1; i < token.length() + 1; i++)
        if (token[i] == ' ' or token[i] == 0)
            break;
        else
            if (not (isalpha(token[0]) or isdigit(token[0]) or token[0] == '_')) return false;
    token[i] = 0;
    return true;
}

template <class T> ValueType TParser<T>::get_exp(TNode<T> &code)
{
    get_token();
    if (not token.length())
        set_error(Error::Syntax);
    return token_or(code);
}

template <class T> ValueType TParser<T>::token_or(TNode<T> &code)
{
    TNode<T> hold;
    ValueType ret = token_and(code);

    while (token_type not_eq TokenType::Finished and tok == Token::Or)
    {
        get_token();
        if (token_and(hold) not_eq ValueType::Scalar or ret not_eq ValueType::Scalar)
            set_error(Error::InvalidOperation);
        code = TNode<T>(make_shared<TNode<T>>(code), Token::Or, make_shared<TNode<T>>(hold));
    }
    return ret;
}

template <class T> ValueType TParser<T>::token_and(TNode<T> &code)
{
    TNode<T> hold;
    ValueType ret = token_not(code);

    while (token_type not_eq TokenType::Finished and tok == Token::And)
    {
        get_token();
        if (token_not(hold) not_eq ValueType::Scalar or ret not_eq ValueType::Scalar)
            set_error(Error::InvalidOperation);
        code = TNode<T>(make_shared<TNode<T>>(code), Token::And, make_shared<TNode<T>>(hold));
    }
    return ret;
}

template <class T> ValueType TParser<T>::token_not(TNode<T> &code)
{
    Token op = tok;
    ValueType ret;

    if (token_type == TokenType::Delimiter and op == Token::Not)
        get_token();
    ret = token_add(code);
    if (op == Token::Not)
    {
        if (ret not_eq ValueType::Scalar)
            set_error(Error::InvalidOperation);
        code = TNode<T>(Token::Not, make_shared<TNode<T>>(code));
    }
    return ret;
}

template <class T> ValueType TParser<T>::token_add(TNode<T> &code)
{
    Token op;
    TNode<T> hold;
    string pm;
    ValueType ret = token_mul(code);

    while (token_type not_eq TokenType::Finished and ((pm = token) == "+" or pm == "-" or pm == ">" or
                                                      pm == "<" or pm == ">=" or pm == "<=" or pm == "<>" or pm == "=="))
    {
        get_token();
        if (token_mul(hold) not_eq ret)
            set_error(Error::InvalidOperation);
        is_find(opeartionList, pm, op);
        code = TNode<T>(make_shared<TNode<T>>(code), op, make_shared<TNode<T>>(hold));
    }
    return ret;
}

template <class T> ValueType TParser<T>::token_mul(TNode<T> &code)
{
    Token  op;
    TNode<T> hold;
    char pm;
    ValueType rl = token_var(code),
              rr;

    while (token_type not_eq TokenType::Finished and ((pm = token[0]) == '*' or pm == '/'))
    {
        op = (pm == '*') ? Token::Mul : Token::Div;
        get_token();
        rr = token_var(hold);
        if (op == Token::Div and rr not_eq ValueType::Scalar)
            set_error(Error::InvalidOperation);
        else if (op == Token::Mul)
        {
            if (rl == ValueType::Scalar)
                rl = rr;
            else if (rr not_eq ValueType::Scalar)
                set_error(Error::InvalidOperation);
        }
        code = TNode<T>(make_shared<TNode<T>>(code), op, make_shared<TNode<T>>(hold));
    }
    return rl;
}

template <class T> ValueType TParser<T>::token_var(TNode<T> &code)
{
    string lhs = token,
           rhs;
    TNode<T> hold;
    ValueType ret_left = token_pow(code),
              ret_right;

    while (token_type not_eq TokenType::Finished and token == "var")
    {
        get_token();
        rhs = token;
        if ((ret_left == ValueType::Scalar or ret_left == ValueType::Vector) and (ret_right = token_pow(hold)) == ValueType::Vector)
        {
            if (ret_left == ValueType::Scalar and (not is_find(load, lhs) or not is_find(result, rhs)))
                set_error(Error::Syntax);
            code = TNode<T>(make_shared<TNode<T>>(code), Token::Variation, make_shared<TNode<T>>(hold));
            ret_left = ValueType::Matrix;
        }
        else
            set_error(Error::InvalidOperation);
    }
    return ret_left;
}

template <class T> ValueType TParser<T>::token_pow(TNode<T>& code)
{
    TNode<T> hold;
    ValueType ret = token_un(code);

    if (token_type not_eq TokenType::Finished and token == "**")
    {
        get_token();
        if (token_bracket(hold) not_eq ValueType::Scalar and ret not_eq ValueType::Scalar)
            set_error(Error::InvalidOperation);
        code = TNode<T>(make_shared<TNode<T>>(code), Token::Pow, make_shared<TNode<T>>(hold));
    }
    return ret;
}

template <class T> ValueType TParser<T>::token_un(TNode<T> &code)
{
    Token op = Token::Indefined;
    ValueType ret;

    if ((token_type == TokenType::Delimiter) and (token[0] == '+' or token[0] == '-'))
    {
        if (token[0] == '+')
            op = Token::Plus;
        else
            op = Token::Minus;
        get_token();
    }
    ret = token_bracket(code);
    if (op not_eq Token::Indefined)
        code = TNode<T>(op, make_shared<TNode<T>>(code));
    return ret;
}

template <class T> ValueType TParser<T>::token_prim(TNode<T> &code)
{
    stringstream s;
    string name;
    double d;
    ValueType ret = ValueType::Scalar;

    switch (token_type)
    {
    case TokenType::Variable:
        name = token;
        get_token();
        if (is_find(result, name))
        {
            code = &(find_if(result.begin(), result.end(), [name](pair<string, TValue<T>> i) { return i.first == name; } )->second);
            ret = ValueType::Vector;
        }
        else if (is_find(constant, name))
        {
            get_value(constant, name, code);
            ret = ValueType::Scalar;
        }
        else if (is_find(load, name))
        {
            get_value(load, name, code);
            ret = ValueType::Scalar;
        }
        else if (is_find(function, name))
        {
            get_value(function, name, code);
            ret = ValueType::Vector;
        }
        else if (is_find(functional, name))
        {
            get_value(functional, name, code);
            ret = ValueType::Matrix;
        }
        else if (is_find(argument, name))
        {
            if (!is_predicate)
                set_error(Error::UsingArgument);
            code = &(find_if(argument.begin(), argument.end(), [name](pair<string, TValue<T>> i) { return i.first == name; } )->second);
            ret = ValueType::Scalar;
        }
        else
            set_error(Error::UndefinedVariable);
        break;
    case TokenType::Number:
        s << token;
        s >> d;
        if (s.fail())
            set_error(Error::Syntax);
        //            val = std::stod(token);
        code = TNode<T>(d);
        get_token();
        ret = ValueType::Scalar;
        break;
    case TokenType::Function:
        ret = token_func(code);
        break;
    default:
        set_error(Error::Syntax);
    }
    return ret;
}

template <class T> ValueType TParser<T>::token_bracket(TNode<T> &code)
{
    ValueType ret;

    if (token[0] == '(' and token_type == TokenType::Delimiter)
    {
        get_token();
        ret = token_or(code);
        if(token[0] not_eq ')')
            set_error(Error::Bracket);
        get_token();
    }
    else
        ret = token_prim(code);
    return ret;
}

template <class T> ValueType TParser<T>::token_func(TNode<T> &code)
{
    Token fun_tok = tok;
    TNode<T> hold;
    ValueType ret = ValueType::Scalar;

    if (token_type == TokenType::Function)
    {
        get_token();
        if (not token.length() or token[0] not_eq '(')
            set_error(Error::Syntax);
        get_token();
        ret = token_add(code);
        if (fun_tok == Token::Atan2)
        {
            if (token[0] not_eq ',')
                set_error(Error::Syntax);
            if (ret not_eq ValueType::Scalar)
                set_error(Error::InvalidOperation);
            get_token();
            token_add(hold);
            code = TNode<T>(make_shared<TNode<T>>(code), Token::Atan2, make_shared<TNode<T>>(hold));
        }
        else if (fun_tok == Token::Diff)
        {
            if (ret not_eq ValueType::Vector)
                set_error(Error::InvalidOperation);
            if (token[0] not_eq ',')
                set_error(Error::Syntax);
            get_token();
            if (token == argument[0].first)
                code = TNode(make_shared<TNode<T>>(code), Token::Diff, make_shared<TNode<T>>(0));
            else if (token == argument[1].first)
                code = TNode(make_shared<TNode<T>>(code), Token::Diff, make_shared<TNode<T>>(1));
            else if (token == argument[2].first)
                code = TNode(make_shared<TNode<T>>(code), Token::Diff, make_shared<TNode<T>>(2));
            else
                set_error(Error::Syntax);
            get_token();
        }
        else
        {
            if (fun_tok not_eq Token::Integral and ret == ValueType::Matrix)
                set_error(Error::InvalidOperation);
            code = TNode<T>(fun_tok, make_shared<TNode<T>>(code));
        }
//        get_token();
        if (token[0] not_eq ')')
            set_error(Error::Syntax);
        get_token();
    }
    return ret;
}

template <class T> void TParser<T>::get_boundary_conditions(TMesh &mesh, list<tuple<int, int, int, double>> &bc)
{
    for (auto i = 0; i < mesh.get_x().size1(); i++)
    {
        for (auto j = 0; j < mesh.get_x().size2(); j++)
            argument[j].second = mesh.get_x(i, j);
        for (auto [name, type, predicate, val]: bc_list)
            if (predicate.value().asScalar() not_eq 0)
                bc.push_back(make_tuple(i, type, (type == 1) ? get_name_no(result, name) : get_name_no(load, name), val.value().asScalar()));
    }
}


#endif //PARSER_H
