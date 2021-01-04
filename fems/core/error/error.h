#ifndef ERROR_H
#define ERROR_H

#include <string>

enum class Error { Undefined = 0, NotSpecifiedProgram, UndefinedVariable, EmptyProgram, Syntax, Bracket, InvalidIdentifier, VariableOverride, AssignmentArgument,
                   AssignmentResult, UsingArgument, InvalidInitialisation, InvalidOperation, MeshFormat, InvalidFE, ReadFile, InternalError, AsScalar,
                   AsVector, AsMatrix, IncorrectFE, NotSolution, InvalidBoundaryCondition, Preprocessor, NotMesh };

class TError
{
private:
    Error error = Error::Undefined;
public:
    TError(void) {}
    TError(Error e) : error{e} {}
    ~TError(void) {}
    std::string say(void)
    {
        switch (error)
        {
        case Error::UndefinedVariable:
            return "Use of undeclared identifier";
        case Error::NotSpecifiedProgram:
            return "Program not specified";
        case Error::EmptyProgram:
            return "Empty program specified";
        case Error::Syntax:
            return "Syntax error";
        case Error::Bracket:
            return "Unbalanced brackets";
        case Error::InvalidIdentifier:
            return "Incorrect identifier name";
        case Error::VariableOverride:
            return "Variable override";
        case Error::AssignmentArgument:
            return "Assigning a value to an argument";
        case Error::AssignmentResult:
            return "Assigning a value to an result function";
        case Error::UsingArgument:
            return "Incorrect use of argument";
        case Error::InvalidInitialisation:
            return "Invalid initialization";
        case Error::InvalidOperation:
            return "Invalid operation";
        case Error::MeshFormat:
            return "Mesh format error";
        case Error::InvalidFE:
            return "Invalid finite elemet";
        case Error::ReadFile:
            return "Open/Read file error";
        case Error::InternalError:
            return "Internal error";
        case Error::AsScalar:
            return "Invalid scalar access";
        case Error::AsVector:
            return "Invalid vector access";
        case Error::AsMatrix:
            return "Invalid matrix access";
        case Error::IncorrectFE:
            return "Incorrect FE";
        case Error::NotSolution:
            return "System of linear equations not have a solution";
        case Error::InvalidBoundaryCondition:
            return "Invalid boundary condition";
        case Error::Preprocessor:
            return "Incorrect format of the specified mesh-file name";
        case Error::NotMesh:
            return "No mesh set";
        default:
            break;
        }
        return "";
    }

};

#endif // ERROR_H
