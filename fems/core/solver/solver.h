#ifndef SOLVER_H
#define SOLVER_H

#include <string>
#include "matrix/matrix.h"

using namespace std;

class TMesh;

template <class T> class TSolver
{
protected:
    T matrix;
    vector<double> loadVector;
    virtual bool loadMatrix(string, T&) = 0;
    virtual bool saveMatrix(string, T&) = 0;
public:
    TSolver(void) {}
    virtual ~TSolver(void) {}
    virtual void clear(void) = 0;
    virtual void setBoundaryCondition(unsigned, double) = 0;
    virtual void setup(TMesh&) = 0;
    virtual void setMatrix(double, unsigned, unsigned) = 0;
    virtual void addMatrix(double, unsigned, unsigned) = 0;
    void setLoad(double value, unsigned i)
    {
        loadVector[i] = value;
    }
    void addLoad(double value, unsigned i)
    {
        loadVector[i] += value;
    }
    double getLoad(unsigned i)
    {
        return loadVector[i];
    }
    virtual double getMatrix(unsigned, unsigned) = 0;
    T& getMatrix(void)
    {
        return matrix;
    }
    vector<double>& getLoadVector(void)
    {
        return loadVector;
    }
    virtual bool solve(vector<double>&, double, bool&) = 0;
    virtual void print(string) = 0;
    bool saveMatrix(string fname)
    {
        return saveMatrix(fname, matrix);
    }
    bool loadMatrix(string fname)
    {
        return loadMatrix(fname, matrix);
    }
};

#endif // SOLVER_H
