#ifndef MATRIX_H
#define MATRIX_H

#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

template <typename T> ostream &operator << (ostream &out, vector<T> &r)
{
    for (auto i = 0u; i < r.size(); i++)
    {
        out << fixed << setprecision(8) << r[i];
        if (i < r.size() - 1)
            out << ' ';
    }
    return out;
}

template <typename T> vector<T> operator + (const vector<T> &lhs, const vector<T> &rhs)
{
    vector<T> r(lhs.size());

    transform(lhs.begin(), lhs.end(), rhs.begin(), r.begin(), plus<T>());
    return r;
}

template <typename T> vector<T> operator - (const vector<T> &lhs, const vector<T> &rhs)
{
    vector<T> r(lhs.size());

    transform(lhs.begin(), lhs.end(), rhs.begin(), r.begin(), minus<T>());
    return r;
}


template <typename T> void operator += (vector<T> &lhs, const vector<T> &rhs)
{
    transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), plus<T>());
}

template <typename T> void operator *= (vector<T> &lhs, T rhs)
{
    for_each(lhs.begin(), lhs.end(), [rhs](auto &i){ i *= rhs; });
}

template <typename T> void operator -= (vector<T> &lhs, const vector<T> &rhs)
{
    transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), minus<T>());
}

template <typename T, typename M> vector<T> operator * (const vector<T> &lhs, M rhs)
{
    vector<T> r{lhs};

    for_each(r.begin(), r.end(), [rhs](auto &i){i *= rhs;});
    return r;
}

template <typename T, typename M> vector<T> operator / (const vector<T> &lhs, M rhs)
{
    vector<T> r{lhs};

    for_each(r.begin(), r.end(), [rhs](auto &i){i /= rhs;});
    return r;
}


template <typename T> vector<T> operator * (T lhs, const vector<T> &rhs)
{
    return rhs * lhs;
}

template <typename T> T sum(const vector<T> &val)
{
    T sum_of_elems {0};

    for_each(val.begin(), val.end(), [&] (T n) { sum_of_elems += n;});
    return sum_of_elems;
}

template <typename T> void inverse(vector<T> &val)
{
    for_each(val.begin(), val.end(), [](auto &i){i = -i;});
}

template <typename T> void norm3(vector<T> &res)
{
    T norm {sqrt(res[0] * res[0] + res[1] * res[1] + res[2] * res[2])};

    for (size_t k = 0; k < 3; k++)
        res[k] /= norm;
}

template <typename T> vector<T> createVector3(T *xi, T *xj)
{
    vector<T> res(3);

    for (size_t k = 0; k < 3; k++)
        res[k] = xj[k] - xi[k];
    norm3(res);
    return res;
}

template <typename T> vector<T> crossProduct3(vector<T> &a, vector<T> &b)
{
    vector<T> res(3);

    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
    norm3(res);
    return res;
}

template <typename T> class matrix
{
private:
    vector<T> buffer;
    size_t rows;
    size_t cols;
public:
    matrix(void)
    {
        rows = cols = 0;
    }
    matrix(size_t r, size_t c)
    {
        rows = r;
        cols = c;
        buffer.resize(rows * cols);
        fill(0);
    }
    matrix(const matrix<T> &r)
    {
        rows = r.rows;
        cols = r.cols;
        buffer = r.buffer;
    }
    matrix(initializer_list<initializer_list<T>> r)
    {
        rows = r.size();
        cols = r.begin()[0].size();
        for (auto i: r)
            for (auto j: i)
                buffer.push_back(j);
    }
    ~matrix() {}
    void resize(size_t r, size_t c)
    {
        rows = r;
        cols = c;
        buffer.resize(rows*cols);
        fill(0);
    }
    matrix operator = (const matrix &r)
    {
        rows = r.rows;
        cols = r.cols;
        buffer = r.buffer;
        return *this;
    }
    void operator += (const matrix &r)
    {
        buffer += r.buffer;
    }
    void operator *= (T rhs)
    {
        buffer *= rhs;
    }
    T *operator [] (size_t i)
    {
        return buffer.data() + i * cols;
    }
    const T *operator [] (size_t i) const
    {
        return buffer.data() + i * cols;
    }
    T &operator () (size_t i, size_t j)
    {
        return (buffer.data() + i * cols)[j];
    }
    const T &operator () (size_t i, size_t j) const
    {
        return (buffer.data() + i * cols)[j];
    }
    size_t size1(void) const
    {
        return rows;
    }
    size_t size2(void) const
    {
        return cols;
    }
    friend ostream &operator << (ostream &out, const matrix &r)
    {
        out.setf( ios::fixed,  ios::floatfield );
        for (auto i = 0u; i < r.size1(); i++)
        {
            for (auto j = 0u; j < r.size2(); j++)
            {
                out.precision(5);
                out.width(15);
                out << r[i][j] << ' ';
            }
            out << endl;
        }
        return out;
    }
    void fill(T val)
    {
        std::fill(buffer.begin(), buffer.end(), val);
    }
    T *data(void)
    {
        return buffer.data();
    }
    vector<T> &asVector(void)
    {
        return buffer;
    }
};

template <typename T> matrix<T> transpose(const vector<T> &v)
{
    matrix<T> res(v.size(), 1);

    for (auto i = 0; i < v.size(); i++)
        res[i][0] = v[i];
    return res;
}


template <typename T> matrix<T> operator + (const matrix<T> &lhs, const matrix<T> &rhs)
{
    matrix<T> res(lhs.size1(), lhs.size2());

    for (auto row = 0u; row < lhs.size1(); row++)
        for (auto col = 0u; col < lhs.size2(); col++)
            res[row][col] = lhs[row][col] + rhs[row][col];
    return res;
}

template <typename T> matrix<T> operator - (const matrix<T> &lhs, const matrix<T> &rhs)
{
    matrix<T> res(lhs.size1(), lhs.size2());

    for (auto row = 0u; row < lhs.size1(); row++)
        for (auto col = 0u; col < lhs.size2(); col++)
            res[row][col] = lhs[row][col] - rhs[row][col];
    return res;
}

template <typename T> void operator += (matrix<T> &lhs, const matrix<T> &rhs)
{
    for (auto row = 0; row < lhs.size1(); row++)
        for (auto col = 0; col < lhs.size2(); col++)
            lhs[row][col] += rhs[row][col];
}

template <typename T> matrix<T> operator * (const matrix<T> &lhs, const matrix<T> &rhs)
{
    matrix<T> res(lhs.size1(), rhs.size2());

    for (auto row = 0; row < lhs.size1(); row++)
        for (auto col = 0; col < rhs.size2(); col++)
            for (auto inner = 0; inner < lhs.size2(); inner++)
                res[row][col] += lhs[row][inner] * rhs[inner][col];
    return res;
}

template <typename T> matrix<T> operator * (const matrix<T> &lhs, const vector<T> &rhs)
{
    matrix<T> res;

    if (lhs.size2() == rhs.size())
    {
        res.resize(lhs.size1(), 1);
        for (auto row = 0; row < lhs.size1(); row++)
            for (auto inner = 0; inner < lhs.size2(); inner++)
                res[row][0] += lhs[row][inner] * rhs[inner];
    }
    else if (lhs.size2() == 1 and lhs.size1() == rhs.size())
    {
        res.resize(lhs.size1(), lhs.size1());
        for (auto row = 0; row < lhs.size1(); row++)
            for (auto col = 0; col < rhs.size(); col++)
                res[row][col] += lhs[row][0] * rhs[col];
    }
    return res;
}

template <typename T, typename M> matrix<T> operator * (const matrix<T> &lhs, M rhs)
{
    matrix<T> res = lhs;

    for (auto row = 0u; row < lhs.size1(); row++)
        for (auto col = 0u; col < lhs.size2(); col++)
            res[row][col] *= rhs;
    return res;
}

template <typename T, typename M> matrix<T> operator * (M lhs, const matrix<T> &rhs)
{
    return rhs * lhs;
}

template <typename T> matrix<T> operator / (const matrix<T> &lhs, T rhs)
{
    matrix<T> res = lhs;

    for (auto row = 0u; row < lhs.size1(); row++)
        for (auto col = 0u; col < lhs.size2(); col++)
            res[row][col] /= rhs;
    return res;
}

template <typename T, typename M> matrix<T> operator / (M lhs, const matrix<T> &rhs)
{
    return rhs / lhs;
}

template <typename T> matrix<T> transpose(const matrix<T> &m)
{
    matrix<T> res(m.size2(), m.size1());

    for (auto i = 0; i < m.size2(); i++)
        for (auto j = 0; j < m.size1(); j++)
            res(i, j) = m(j, i);
    return res;
}

template <typename T> T det(const matrix<T> &m)
{
    assert(m.size1() == m.size2() and m.size1() < 4);
    return m.size1() == 1 ? m[0][0] : m.size1() == 2 ? det2x2(m) : det3x3(m);
}


template <typename T> T det2x2(const matrix<T> &m)
{
    return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
}

template <typename T> T det3x3(const matrix<T> &m)
{
    return m(0, 0) * m(1, 1) * m(2, 2) + m(0, 1) * m(1, 2) * m(2, 0) + m(0, 2) * m(1, 0) * m(2, 1) -
           m(0, 2) * m(1, 1) * m(2, 0) - m(0, 0) * m(1, 2) * m(2, 1) - m(0, 1) * m(1, 0) * m(2, 2);
}

template <typename T> matrix<T> inv(const matrix<T> &m)
{
    matrix<T> res(m.size1(), m.size2());

    assert(m.size1() == m.size2() and m.size1() < 4);
    if (m.size1() == 1)
        res(0, 0) = 1.0 / m(0, 0);
    return m.size1() == 1 ? res : m.size1() == 2 ? inv2x2(m) : inv3x3(m);
}

template <typename T> matrix<T> inv2x2(const matrix<T> &m)
{
    matrix<T> res(2, 2);

    res(0, 0) =  m(1, 1); res(0, 1) = -m(0, 1);
    res(1, 0) = -m(1, 0); res(1, 1) = m(0, 0);
    return res / det2x2(m);
}

template <typename T> matrix<T> inv3x3(const matrix<T> &m)
{
    matrix<T> res(3, 3);

    res(0, 0) = m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1); res(0, 1) = m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2); res(0, 2) = m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1);
    res(1, 0) = m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2); res(1, 1) = m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0); res(1, 2) = m(0, 2) * m(1, 0) - m(0, 0) * m(1, 2);
    res(2, 0) = m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0); res(2, 1) = m(0, 1) * m(2, 0) - m(0, 0) * m(2, 1); res(2, 2) = m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);

    return res / det3x3(m);
}

//--------------------------------------------------------------
//                  Решение СЛАУ методом Гаусса
//--------------------------------------------------------------
inline bool solve(matrix<double> &matr, vector<double> &result, double eps = 1.0E-20)
{
    double val;

    for (auto i = 0u; i < matr.size1() - 1; i++)
    {
        if (fabs(matr[i][i]) == 0)
            for (auto l = i + 1; l < matr.size1(); l++)
                if (fabs(matr[l][i]) > eps)
                    for (auto j = 0u; j < matr.size1() + 1; j++)
                        swap(matr[l][j], matr[i][j]);
        for (auto j = i + 1u; j < matr.size1(); j++)
        {
            val = matr[j][i] / matr[i][i];
            for (auto k = i; k < matr.size1() + 1u; k++)
                matr[j][k] -= matr[i][k] * val;
        }
    }
    if (fabs(matr[matr.size1() - 1][matr.size1() - 1]) < eps)
        return false;
    result[matr.size1() - 1] = matr[matr.size1() - 1][matr.size1()] / matr[matr.size1() - 1][matr.size1() - 1];
    for (auto k = 0u; k < matr.size1() - 1; k++)
    {
         auto i = matr.size1() - k - 2;

         val = matr[i][matr.size1()];
         for (auto j = i + 1u; j < matr.size1(); j++)
             val -= result[j] * matr[i][j];
         if (fabs(matr[i][i]) < eps)
             return false;
         result[i] = val / matr[i][i];
    }
    return true;
}


#endif // MATRIX_H
