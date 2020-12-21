#ifndef EIGENSOLVER_H
#define EIGENSOLVER_H

#include <mutex>
#include <Eigen/Sparse>
#include "solver.h"

using namespace Eigen;
using namespace std;

class TMesh;

class TEigenSolver : public TSolver<SparseMatrix<double>>
{
private:
    VectorXi memMap;
    mutex mtx;
    bool loadMatrix(string, SparseMatrix<double>&);
    bool saveMatrix(string, SparseMatrix<double>&);
public:
    TEigenSolver(void) {}
    virtual ~TEigenSolver(void) {}
    void setup(TMesh*);
    void setBoundaryCondition(unsigned, double);
    void clear(void)
    {
        matrix.resize(0, 0);
        memMap.resize(0);
        loadVector.clear();
    }
    void product(SparseMatrix<double>&, vector<double>&, vector<double>&);
    void setMatrix(double value, unsigned i, unsigned j)
    {
        matrix.coeffRef(i, j) = value;
    }
    void addMatrix(double value, unsigned i, unsigned j)
    {
        lock_guard<mutex> guard(mtx);
        matrix.coeffRef(i, j) += value;
    }
    void print(string);
    double getMatrix(unsigned i, unsigned j)
    {
        return matrix.coeff(i, j);
    }
    bool solve(vector<double>&, double, bool&);
};

#endif // EIGENSOLVER_H
