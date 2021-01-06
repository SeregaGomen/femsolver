#include "solver/eigensolver.h"
#include "fem/fem.h"

using namespace std;

int main(int argc, char **argv)
{
    TFEM<TEigenSolver> fem;

    try
    {
        if (argc < 2)
            throw TError(Message::NotSpecifiedProgram);
        fem.set_program(argv[1]);
        fem.start();
    }
    catch (TError &e)
    {
        cerr << e.say() << endl;
        return 1;
    }


    return 0;
}
