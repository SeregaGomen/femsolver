#ifndef MSG_H
#define MSG_H

#include <string>
#include <sstream>
#include <thread>
#include <iostream>
#include <vector>
#include <iomanip>

enum class Message { Undefined = 0, NotSpecifiedProgram, UndefinedVariable, EmptyProgram, Syntax, Bracket, InvalidIdentifier, VariableOverride, AssignmentArgument,
                     AssignmentResult, UsingArgument, InvalidInitialisation, InvalidOperation, MeshFormat, InvalidFE, ReadFile, InternalError, AsScalar,
                     AsVector, AsMatrix, IncorrectFE, NotSolution, InvalidBoundaryCondition, Preprocessor, NotMesh,

                     GeneratingMatrix, UsingBoundaryCondition, PreparingSystemEquation, FactorizationSystemEquation, SolutionSystemEquation, AnalysingMesh, WritingResult,
                     GeneratingResult, Timer, Sec, FEType, FE1D2, FE2D3, FE2D4, FE2D6, FE3D4, FE3D8, FE3D10, FE2D3P, FE2D4P, FE2D6P, FE3D3S, FE3D4S, FE3D6S, NumNodes,
                     NumFE };


using namespace std;

inline string say_message(Message msg)
{
    vector<pair<Message, string>> msg_table { { Message::UndefinedVariable, "Use of undeclared identifier" }, { Message::NotSpecifiedProgram, "Program not specified" },
                                              { Message::EmptyProgram, "Empty program specified" }, { Message::Syntax, "Syntax error" },
                                              { Message::Bracket, "Unbalanced brackets" }, { Message::InvalidIdentifier, "Incorrect identifier name" },
                                              { Message::VariableOverride, "Variable override" }, { Message::AssignmentArgument, "Assigning a value to an argument" },
                                              { Message::AssignmentResult, "Assigning a value to an result function" }, { Message::UsingArgument, "Incorrect use of argument" },
                                              { Message::InvalidInitialisation, "Invalid initialization" }, { Message::InvalidOperation, "Invalid operation" },
                                              { Message::MeshFormat, "Mesh format error" }, { Message::InvalidFE, "Invalid finite elemet" },
                                              { Message::ReadFile, "Open/Read file error" }, { Message::InternalError, "Internal error" },
                                              { Message::AsScalar, "Invalid scalar access" }, { Message::AsVector, "Invalid vector access" },
                                              { Message::AsMatrix, "Invalid matrix access" }, { Message::IncorrectFE, "Incorrect FE" },
                                              { Message::NotSolution, "System of linear equations not have a solution" }, { Message::InvalidBoundaryCondition, "Invalid boundary condition" },
                                              { Message::Preprocessor, "Incorrect format of the specified mesh-file name" }, { Message::NotMesh, "No mesh set" },
                                              { Message::Timer, "Done in: " }, { Message::Sec, " sec." }, { Message::AnalysingMesh, "Analysing of the mesh structure" },
                                              { Message::GeneratingMatrix, "Building a global stiffness matrix" }, { Message::UsingBoundaryCondition, "Using of boundary conditions" },
                                              { Message::PreparingSystemEquation, "Preparing the system of equations" }, { Message::SolutionSystemEquation, "Solution of the system of equations" },
                                              { Message::FEType, "FE type - " }, { Message::FE1D2, "one-dimensional linear element (2 nodes)" }, { Message::FE2D3,"linear triangular element (3 nodes)" },
                                              { Message::FE2D4, "quadrilateral element (4 nodes)" }, { Message::FE2D6, "quadratic triangular element (6 nodes)" },
                                              { Message::FE3D4, "linear tetrahedron (4 nodes)" },  { Message::FE3D8, "cube element (8 nodes)" },
                                              { Message::FE3D10, "quadratic tetrahedron (10 nodes)" }, { Message::FE2D3P, "plate triangular element (3 nodes)" },
                                              { Message::FE2D4P, "plate quadrilateral element (4 nodes)" }, { Message::FE2D6P, "plate quadrilateral element (6 nodes)" },
                                              { Message::FE3D3S, "shell triangular element (3 nodes)" }, { Message::FE3D4S, "shell quadrilateral element (4 nodes)" },
                                              { Message::FE3D6S, "shell triangular element (6 nodes)" }, { Message::NumNodes, "Number of nodes - " },
                                              { Message::NumFE, "Number of finite elements - " }, { Message::WritingResult, "Writing results" },
                                              { Message::GeneratingResult, "Calculation of results" } };

    return find_if(msg_table.begin(), msg_table.end(), [msg](pair<Message, string> i) { return i.first == msg; } )->second;
}

class TError
{
private:
    Message msg = Message::Undefined;
public:
    TError(void) {}
    TError(Message m) : msg{m} {}
    ~TError(void) {}
    inline string say(void)
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

        ss << '\r' << say_message(process_code) << "... 100%" << endl << say_message(Message::Timer) << setprecision(2) << double(static_cast<chrono::duration<double>>(chrono::system_clock::now() - timer).count()) << say_message(Message::Sec) << endl;
        cout << ss.str() << flush;
    }
    virtual void stop(void)
    {
        is_stopped = true;
        this_thread::sleep_for(std::chrono::milliseconds(200));
        cout << say_message(Message::Timer) << setprecision(3) << double((static_cast< chrono::duration<double> >(chrono::system_clock::now() - timer).count())) << say_message(Message::Sec) << endl;
    }
};

#endif // MSG_H
