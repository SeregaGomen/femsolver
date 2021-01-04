#ifndef SHAPE_H
#define SHAPE_H

#include <iostream>
#include <algorithm>
#include <array>
#include "matrix/matrix.h"
#include "error/error.h"

using namespace std;

enum class Direct { X, Y, Z };

// Параметры функции формы линейного стержневого конечного элемента
// N(x) = c0 + c1 * x
class TShape1d2
{
public:
    TShape1d2(void) noexcept = default;
    ~TShape1d2(void) noexcept = default;
    // Дифференцирование
    static vector<double> diff(const vector<double> &c, Direct)
    {
        return { c[1], 0 };
    }
    static double value(const vector<double> &c, const array<double, 3> &x) noexcept
    {
        return (c.size() == 1) ? c[0] : c[0] + c[1] * x[0];
    }
    static matrix<double> jacobi(int, const matrix<double> &x)
    {
        return { { (x(1, 0) - x(0, 0)) * 0.5 } };
    }
    static int size(void) noexcept
    {
        return 2;
    }
    static int freedom(void) noexcept
    {
        return 1;
    }
    static vector<double> w(void)
    {
        return { 0.555555555566666, 0.888888888888889, 0.555555555566666 };
    }
    static vector<double> xi(void)
    {
        return { -0.774596669241483, 0, 0.774596669241483 };
    }
    static vector<double> eta(void)
    {
        return { 0, 0, 0 };
    }
    static vector<double> psi(void)
    {
        return { 0, 0, 0 };
    }
    static double coeff(matrix<double> &x, int i, int j)
    {
        return vector<double>{ 1.0, x(i, 0) }[j];
    }
    static double x(int i, const matrix<double> &ij)
    {
        return ij(0, 0) * xi()[i];
    }
    static double y(int, const matrix<double>&)
    {
        return 0;
    }
    static double z(int, const matrix<double>&)
    {
        return 0;
    }
};

// Параметры функции формы линейного треугольного конечного элемента
// N(x, y) = c0 + c1 * x + c2 * y
class TShape2d3
{
public:
    TShape2d3(void) noexcept = default;
    ~TShape2d3(void) noexcept = default;
    // Дифференцирование
    static vector<double> diff(const vector<double> &c, Direct direct)
    {
        switch (direct)
        {
        case Direct::X:
            return {c[1], 0, 0};
            break;
        case Direct::Y:
            return {c[2], 0, 0};
            break;
        default:
            throw TError(Error::InternalError);
        }
        return {};
    }
    static double value(const vector<double> &c, const array<double, 3> &x) noexcept
    {
        return (c.size() == 1) ? c[0] : c[0] + c[1] * x[0] + c[2] * x[1];
    }
    static matrix<double> jacobi(int, const matrix<double> &x)
    {
        matrix<double> jacobi(2, 2);
        vector<double> d_xi{ -1.0, 1.0, 0.0 },
                       d_eta{ -1.0, 0.0, 1.0 };

        for (auto j = 0; j < 2; j++)
            for (auto k = 0; k < size(); k++)
            {
                jacobi(0, j) += d_xi[k] * x(k, j);
                jacobi(1, j) += d_eta[k] * x(k, j);
            }
        return jacobi;
    }
    static int size(void) noexcept
    {
        return 3;
    }
    static int freedom(void) noexcept
    {
        return 2;
    }
    static vector<double> w(void)
    {
        return { 0.166666666667, 0.166666666667, 0.166666666667 };
    }
    static vector<double> xi(void)
    {
        return { 0.0, 0.5, 0.5 };
    }
    static vector<double> eta(void)
    {
        return { 0.5, 0.0, 0.5 };
    }
    static vector<double> psi(void)
    {
        return {0, 0, 0};
    }
    static double coeff(matrix<double> &x, int i, int j)
    {
        return vector<double>{ 1.0, x(i, 0), x(i, 1) }[j];
    }
    static double x(int i, const matrix<double> &ij)
    {
        return ij(0, 0) * xi()[i] + ij(0, 1) * eta()[i];
    }
    static double y(int i, const matrix<double> &ij)
    {
        return ij(1, 0) * xi()[i] + ij(1, 1) * eta()[i];
    }
    static double z(int, const matrix<double>&)
    {
        return 0;
    }
};


// Класс функции формы
template <class T> class TShape
{
private:
    vector<double> c; // Коэффициенты функции формы
public:
    TShape(double p = 0) noexcept
    {
        c.push_back(p);
    }
    TShape(const vector<double> &p) noexcept : c{p} {}
    TShape(const TShape &p) noexcept : c{p.c} {}
    ~TShape(void) noexcept = default;
    TShape &operator = (const TShape& rhs) noexcept
    {
        c = rhs.c;
        return *this;
    }
    TShape diff(Direct dir)
    {
        return TShape(T::diff(c, dir));
    }
    double value(const array<double, 3> &x) noexcept
    {
        return T::value(c, x);
    }
    static matrix<double> jacobi(int i, const matrix<double> &x)
    {
        return T::jacobi(i, x);
    }
    static int size(void) noexcept
    {
        return T::size();
    }
    static int freedom(void) noexcept
    {
        return T::freedom();
    }
    static vector<double> w(void)
    {
        return T::w();
    }
    static vector<double> xi(void)
    {
        return T::xi();
    }
    static vector<double> eta(void)
    {
        return T::eta();
    }
    static vector<double> psi(void)
    {
        return T::psi();
    }
    static double coeff(matrix<double> &x, int i, int j)
    {
        return T::coeff(x, i, j);
    }
    static double x(int i, const matrix<double> &ij)
    {
        return T::x(i, ij);
    }
    static double y(int i, const matrix<double> &ij)
    {
        return T::y(i, ij);
    }
    static double z(int i, const matrix<double> &ij)
    {
        return T::z(i, ij);
    }
    friend TShape operator - (const TShape &rhs)
    {
        TShape ret(rhs);

        for_each(ret.c.begin(), ret.c.end(), [](auto &i){ i = -i; });
        return ret;
    }
    friend ostream &operator << (ostream &out, const TShape &rhs)
    {
        out << "( ";
        for (auto i: rhs.c)
            out << i << ' ';
        out << ')';
        return out;
    }
};

#endif // SHAPE_H
