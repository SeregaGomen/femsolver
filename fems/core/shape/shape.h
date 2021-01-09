#ifndef SHAPE_H
#define SHAPE_H

#include <iostream>
#include <algorithm>
#include <array>
#include "matrix/matrix.h"
#include "msg/msg.h"

using namespace std;

enum class Direct { X, Y, Z };

// Параметры функции формы линейного стержневого конечного элемента
// N(x) = c0 + c1 * x
struct TShape1d2
{
    // Дифференцирование
    inline static vector<double> diff(const vector<double> &c, Direct direct)
    {
        if (direct not_eq Direct::X)
            throw TError(Message::InternalError);
        return { c[1], 0 };
    }
    inline static double value(const vector<double> &c, const array<double, 3> &x) noexcept
    {
        return (c.size() == 1) ? c[0] : c[0] + c[1] * x[0];
    }
    inline static matrix<double> jacobi(int, const matrix<double> &x)
    {
        return { { (x(1, 0) - x(0, 0)) * 0.5 } };
    }
    inline static constexpr int size(void) noexcept
    {
        return 2;
    }
    inline static constexpr int freedom(void) noexcept
    {
        return 1;
    }
    inline static constexpr int quadrature_degree(void)
    {
        return 3;
    }
    inline static double w(int i)
    {
        return array<double, 3>{ 0.555555555566666, 0.888888888888889, 0.555555555566666 }[i];
    }
    inline static double xi(int i)
    {
        return array<double, 3>{ -0.774596669241483, 0, 0.774596669241483 }[i];
    }
    inline static double coeff(matrix<double> &x, int i, int j)
    {
        return array<double, 2>{ 1.0, x(i, 0) }[j];
    }
    inline static array<double, 3> x(int i, const matrix<double> &x)
    {
        return { 0.5 * x(0, 0) * (1.0 - xi(i)) + 0.5 * x(1, 0) * (1.0 + xi(i)), 0, 0 };
    }
};

// Параметры функции формы линейного треугольного конечного элемента
// N(x, y) = c0 + c1 * x + c2 * y
struct TShape2d3
{
    // Дифференцирование
    inline static vector<double> diff(const vector<double> &c, Direct direct)
    {
        vector<double> ret;

        if (direct == Direct::X)
            ret = {c[1], 0, 0};
        else if (direct == Direct::Y)
            ret = {c[2], 0, 0};
        else
            throw TError(Message::InternalError);
        return ret;
    }
    inline static double value(const vector<double> &c, const array<double, 3> &x) noexcept
    {
        return (c.size() == 1) ? c[0] : c[0] + c[1] * x[0] + c[2] * x[1];
    }
    inline static matrix<double> jacobi(int, const matrix<double> &x)
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
    inline static constexpr int size(void) noexcept
    {
        return 3;
    }
    inline static constexpr int freedom(void) noexcept
    {
        return 2;
    }
    inline static constexpr int quadrature_degree(void)
    {
        return 3;
    }
    inline static double w(int i)
    {
        return array<double, 3>{ 0.166666666667, 0.166666666667, 0.166666666667 }[i];
    }
    inline static double xi(int i)
    {
        return array<double, 3>{ 0.0, 0.5, 0.5 }[i];
    }
    inline static double eta(int i)
    {
        return array<double, 3>{ 0.5, 0.0, 0.5 }[i];
    }
    inline static double coeff(matrix<double> &x, int i, int j)
    {
        return vector<double>{ 1.0, x(i, 0), x(i, 1) }[j];
    }
    inline static array<double, 3> x(int i, const matrix<double> &x)
    {
        return { x(0, 0) * (1.0 - xi(i) - eta(i)) + x(1, 0) * xi(i) + x(2, 0) * eta(i), x(0, 1) * (1.0 - xi(i) - eta(i)) + x(1, 1) * xi(i) + x(2, 1) * eta(i), 0 };
    }
};

// Параметры функции формы линейного тетраэдра
// N(x, y) = c0 + c1 * x + c2 * y
struct TShape3d4
{
    // Дифференцирование
    inline static vector<double> diff(const vector<double> &c, Direct direct)
    {
        vector<double> ret;

        if (direct == Direct::X)
            ret = {c[1], 0, 0, 0};
        else if (direct == Direct::Y)
            ret = {c[2], 0, 0, 0};
        else if (direct == Direct::Z)
            ret = {c[3], 0, 0, 0};
        else
            throw TError(Message::InternalError);
        return ret;
    }
    inline static double value(const vector<double> &c, const array<double, 3> &x) noexcept
    {
        return (c.size() == 1) ? c[0] : c[0] + c[1] * x[0] + c[2] * x[1] + c[3] * x[2];
    }
    inline static matrix<double> jacobi(int, const matrix<double> &x)
    {
        matrix<double> jacobi(3, 3);
        vector<double> d_xi{ -1.0, 1.0, 0.0, 0.0 },
                       d_eta{ -1.0, 0.0, 1.0, 0.0 },
                       d_psi{ -1.0, 0.0, 0.0, 1.0 };

        for (auto j = 0; j < 3; j++)
            for (auto k = 0; k < size(); k++)
            {
                jacobi(0, j) += d_xi[k] * x(k, j);
                jacobi(1, j) += d_eta[k] * x(k, j);
                jacobi(2, j) += d_psi[k] * x(k, j);
            }
        return jacobi;
    }
    inline static constexpr int size(void) noexcept
    {
        return 4;
    }
    inline static constexpr int freedom(void) noexcept
    {
        return 3;
    }
    inline static constexpr int quadrature_degree(void)
    {
        return 5;
    }
    inline static double w(int i)
    {
        return array<double, 5>{ -0.13333333333, 0.075, 0.075, 0.075, 0.075 }[i];
    }
    inline static double xi(int i)
    {
        return array<double, 5>{ 0.25, 0.5, 0.16666666667, 0.16666666667, 0.16666666667 }[i];
    }
    inline static double eta(int i)
    {
        return array<double, 5>{ 0.25, 0.16666666667, 0.5, 0.16666666667, 0.16666666667 }[i];
    }
    inline static double psi(int i)
    {
        return array<double, 5>{ 0.25, 0.16666666667, 0.16666666667, 0.5, 0.16666666667 }[i];
    }
    inline static double coeff(matrix<double> &x, int i, int j)
    {
        return vector<double>{ 1.0, x(i, 0), x(i, 1), x(i, 2) }[j];
    }
    inline static array<double, 3> x(int i, const matrix<double> &x)
    {
        return { x(0, 0) * (1.0 - xi(i) - eta(i) - psi(i)) + x(1, 0) * xi(i) + x(2, 0) * eta(i) + x(3, 0) * psi(i),
                 x(0, 1) * (1.0 - xi(i) - eta(i) - psi(i)) + x(1, 1) * xi(i) + x(2, 1) * eta(i) + x(3, 1) * psi(i),
                 x(0, 2) * (1.0 - xi(i) - eta(i) - psi(i)) + x(1, 2) * xi(i) + x(2, 2) * eta(i) + x(3, 2) * psi(i) };
    }
};


// Класс функции формы
template <class T> class TShape
{
private:
    vector<double> c; // Коэффициенты функции формы
public:
    TShape(double p = 0) noexcept : c{p} {}
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
    inline static constexpr int quadrature_degree(void)
    {
        return T::quadrature_degree();
    }
    inline static constexpr int size(void) noexcept
    {
        return T::size();
    }
    inline static constexpr int freedom(void) noexcept
    {
        return T::freedom();
    }
    inline static double w(int i)
    {
        return T::w(i);
    }
    inline static double coeff(matrix<double> &x, int i, int j)
    {
        return T::coeff(x, i, j);
    }
    inline static array<double, 3> x(int i, const matrix<double> &ij)
    {
        return T::x(i, ij);
    }
    friend TShape operator - (const TShape &rhs)
    {
        TShape ret(rhs);

        for_each(ret.c.begin(), ret.c.end(), [](auto &i){ i = -i; });
        return ret;
    }
    friend TShape operator * (const TShape &lhs, double rhs)
    {
        TShape ret(lhs);

        for_each(ret.c.begin(), ret.c.end(), [rhs](auto &i){ i *= rhs; });
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
