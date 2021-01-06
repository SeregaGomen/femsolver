#ifndef MSG_H
#define MSG_H

#include <string>
#include <sstream>
#include <thread>
#include <iostream>

enum class Message { Undefined = 0, NotSpecifiedProgram, UndefinedVariable, EmptyProgram, Syntax, Bracket, InvalidIdentifier, VariableOverride, AssignmentArgument,
                     AssignmentResult, UsingArgument, InvalidInitialisation, InvalidOperation, MeshFormat, InvalidFE, ReadFile, InternalError, AsScalar,
                     AsVector, AsMatrix, IncorrectFE, NotSolution, InvalidBoundaryCondition, Preprocessor, NotMesh,

                     GeneratingMatrix, UsingBoundaryCondition, PreparingSystemEquation, FactorizationSystemEquation, SolutionSystemEquation, AnalysingMesh,
                     WritingResult, GeneratingResult, Timer, Sec };


using namespace std;

inline string say_message(Message msg)
{
    switch (msg)
    {
    case Message::UndefinedVariable:
        return "Use of undeclared identifier";
    case Message::NotSpecifiedProgram:
        return "Program not specified";
    case Message::EmptyProgram:
        return "Empty program specified";
    case Message::Syntax:
        return "Syntax error";
    case Message::Bracket:
        return "Unbalanced brackets";
    case Message::InvalidIdentifier:
        return "Incorrect identifier name";
    case Message::VariableOverride:
        return "Variable override";
    case Message::AssignmentArgument:
        return "Assigning a value to an argument";
    case Message::AssignmentResult:
        return "Assigning a value to an result function";
    case Message::UsingArgument:
        return "Incorrect use of argument";
    case Message::InvalidInitialisation:
        return "Invalid initialization";
    case Message::InvalidOperation:
        return "Invalid operation";
    case Message::MeshFormat:
        return "Mesh format error";
    case Message::InvalidFE:
        return "Invalid finite elemet";
    case Message::ReadFile:
        return "Open/Read file error";
    case Message::InternalError:
        return "Internal error";
    case Message::AsScalar:
        return "Invalid scalar access";
    case Message::AsVector:
        return "Invalid vector access";
    case Message::AsMatrix:
        return "Invalid matrix access";
    case Message::IncorrectFE:
        return "Incorrect FE";
    case Message::NotSolution:
        return "System of linear equations not have a solution";
    case Message::InvalidBoundaryCondition:
        return "Invalid boundary condition";
    case Message::Preprocessor:
        return "Incorrect format of the specified mesh-file name";
    case Message::NotMesh:
        return "No mesh set";
    case Message::Timer:
        return "Done in: ";
    case Message::Sec:
        return " sec.";
    case Message::AnalysingMesh:
        return "Analysing of the mesh structure";
    case Message::GeneratingMatrix:
        return "Building a global stiffness matrix";
    case Message::UsingBoundaryCondition:
        return "Using of boundary conditions";
    case Message::PreparingSystemEquation:
        return "Preparing the system of equations";
    case Message::SolutionSystemEquation:
        return "Solution of the system of equations";
    default:
        break;
    }
    return "Undefined message";
}

class TError
{
private:
    Message msg = Message::Undefined;
public:
    TError(void) {}
    TError(Message m) : msg{m} {}
    ~TError(void) {}
    string say(void)
    {
        return say_message(msg);
    }
};


class TProgress
{
private:
    chrono::system_clock::time_point timer;
    bool is_stopped = false;
    thread progress_thread;
    void backgroundRun(bool& isStopped)
    {
        char chr[] = { '|', '/', '-', '\\' };
        int i = 0;

        while (not isStopped)
        {
            cout << '\r' << say_message(process_code) << "... " << chr[i++ % 4] << flush;
            //std::this_thread::yield();
            this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        cout << '\r' << say_message(process_code) << "... 100%" << endl;
    }
protected:
    Message process_code;
    int process_start;
    int process_stop;
    int process_current;
    int process_step;
    int old_persent;
public:
    TProgress(void)
    {
        process_code = Message::Undefined;
        process_start = process_stop = process_current = old_persent = 0;
        process_step = 1;
    }
    virtual ~TProgress(void) {}
    virtual void set_process(Message code)
    {
        is_stopped = false;
        process_code = code;
        process_start = process_stop = process_current = old_persent = 0;

        progress_thread = thread(&TProgress::backgroundRun, this, ref(this->is_stopped));
        progress_thread.detach();
        timer = chrono::system_clock::now();
    }
    virtual void set_process(Message code, int start, int stop, int step = 1)
    {
        process_code = code;
        process_start = start;
        process_stop = stop;
        process_step = step;
        process_current = old_persent = 0;
        cout << '\r' << say_message(process_code) << "... 0%" << flush;
        timer = chrono::system_clock::now();
    }
    virtual void add_progress(void)
    {
        stringstream ss;
        int persent = (process_stop - process_start) ? int((100.0 * double(++process_current)) / double(process_stop - process_start)) : 100;

        if (process_current == process_stop)
        {
            ss << '\r' << say_message(process_code) << "... 100%";
            cout << ss.str() << flush;
            return;
        }
        if (persent == old_persent)
            return;
        if (persent % process_step == 0)
        {
            ss << '\r' << say_message(process_code) << "... " << persent << "%";
            cout << ss.str() << flush;
        }
        old_persent = persent;
    }
    void stop_process(void)
    {
        stringstream ss;

        ss << '\r' << say_message(process_code) << "... 100%" << endl << say_message(Message::Timer) << int(double(static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count())) << say_message(Message::Sec) << endl;
        cout << ss.str() << flush;
    }
    virtual void stop(void)
    {
        is_stopped = true;
        this_thread::sleep_for(std::chrono::milliseconds(200));
        cout << say_message(Message::Timer) << int(double((static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count()))) << say_message(Message::Sec) << endl;
    }
};

#endif // MSG_H
