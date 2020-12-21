#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <cmath>
#include <algorithm>
#include <variant>
#include <exception>
#include "shape/shape.h"

using namespace std;

template <class T> class TValue
{
using Scalar = double;
using Vector = variant<vector<T>, vector<double>>;
using Matrix = matrix<double>;

private:
    variant<Scalar, Vector, Matrix> val;
public:
    static array<double, 3> x;
    TValue(double d = 0) noexcept : val{d}  {}
    TValue(const TValue &rhs) noexcept: val{rhs.val} {}
    TValue(const vector<double> &s) noexcept: val{s} {}
    TValue(const vector<T> &s) noexcept: val{s} {}
    TValue(const matrix<double> &s) noexcept: val{s} {}
    ~TValue(void) noexcept = default ;
    TValue value(void) const noexcept
    {
        vector<double> res;

        if (get_if<Vector>(&val) && get_if<vector<T>>(&get<Vector>(val)))
        {
            for (auto i: get<vector<T>>(get<Vector>(val)))
                res.push_back(i.value(x));
            return TValue(res);
        }
        return *this;
    }
    TValue &operator = (const TValue &rhs) noexcept
    {
        val = rhs.val;
        return *this;
    }
    double asScalar(void)
    {
        if (not get_if<Scalar>(&val))
            throw TError(Error::AsScalar);
        return get<Scalar>(val);
    }
    vector<double> asVector(void)
    {
        vector<double> res;

        if (not get_if<Vector>(&val))
            throw TError(Error::AsVector);
        if (get_if<vector<T>>(&get<Vector>(val)))
            for (auto i: get<vector<T>>(get<Vector>(val)))
                res.push_back(i.value(x));
        else
            res = get<vector<double>>(get<Vector>(val));
        return res;
    }
    matrix<double> asMatrix(void)
    {
        if (not get_if<Matrix>(&val))
            throw TError(Error::AsMatrix);
        return get<Matrix>(val);
    }
    friend TValue operator - (const TValue &rhs) noexcept
    {
        TValue ret(rhs);

        if (get_if<Scalar>(&ret.val))
            get<Scalar>(ret.val) = -get<Scalar>(ret.val);
        else if (get_if<Vector>(&ret.val))
        {
            if (get_if<vector<T>>(&get<Vector>(ret.val)))
                inverse(get<vector<T>>(get<Vector>(ret.val)));
            else
                inverse(get<vector<double>>(get<Vector>(ret.val)));
        }
        else
            inverse(get<Matrix>(ret.val).asVector());
        return ret;
    }
    friend TValue diff(const TValue &lhs, const TValue &rhs)
    {
        TValue ret(lhs);
        Direct dir;

        if (not get_if<vector<T>>(&get<Vector>(ret.val)) or not get_if<Scalar>(&rhs.val))
            throw TError(Error::InvalidOperation);
        dir = static_cast<Direct>(get<Scalar>(rhs.val));
        for (auto &i: get<vector<T>>(get<Vector>(ret.val)))
            i = i.diff(dir);
        return ret;
    }
    friend TValue var(const TValue &lhs, const TValue &rhs)
    {
        TValue l{lhs.value()},
               r{rhs.value()};
        matrix<double> res;

        if (not get_if<Vector>(&lhs.val) or not get_if<Vector>(&rhs.val))
            throw TError(Error::InvalidOperation);
        res.resize(get<vector<double>>(get<Vector>(l.val)).size(), get<vector<double>>(get<Vector>(l.val)).size());
        for (auto i = 0u; i < res.size1(); i++)
            for (auto j = 0u; j < res.size1(); j++)
                res[i][j] = get<vector<double>>(get<Vector>(l.val))[i] * get<vector<double>>(get<Vector>(r.val))[j] +
                            get<vector<double>>(get<Vector>(l.val))[j] * get<vector<double>>(get<Vector>(r.val))[i];
        return TValue(res);
    }
    friend ostream &operator << (ostream &out, const TValue &rhs)
    {
        if (get_if<Scalar>(&rhs.val))
            out << "Scalar: " << get<Scalar>(rhs.val);
        else if (get_if<Vector>(&rhs.val))
        {
            out << "Vector: { ";
            if (get_if<vector<T>>(&get<Vector>(rhs.val)))
                for (auto &i: get<vector<T>>(get<Vector>(rhs.val)))
                    out << i << ' ';
            else
                for (auto &i: get<vector<double>>(get<Vector>(rhs.val)))
                    out << i << ' ';
            out << "}";
        }
        else
        {
            out << "Matrix: [ ";
            for (auto i = 0u; i < get<Matrix>(rhs.val).size1(); i++)
            {
                for (auto j = 0u; j < get<Matrix>(rhs.val).size2(); j++)
                    out << get<Matrix>(rhs.val)[i][j] << ' ';
                out << endl;
            }
            out << "]";
        }
        return out;
    }
    friend TValue operator + (const TValue &lhs, const TValue &rhs)
    {
        TValue res;

        if (get_if<Scalar>(&lhs.val) and get_if<Scalar>(&rhs.val))
            res.val = get<Scalar>(lhs.val) + get<Scalar>(rhs.val);
        else if (get_if<Vector>(&lhs.val) and get_if<Vector>(&rhs.val))
            res.val = get<vector<double>>(get<Vector>(lhs.value().val)) + get<vector<double>>(get<Vector>(rhs.value().val));
        else if (get_if<Matrix>(&lhs.val) and get_if<Matrix>(&rhs.val))
            res.val = get<Matrix>(lhs.val) + get<Matrix>(rhs.val);
        else
            throw TError(Error::InvalidOperation);
        return res;
    }
    friend TValue operator - (const TValue &lhs, const TValue &rhs)
    {
        TValue res;

        if (get_if<double>(&lhs.val) and get_if<double>(&rhs.val))
            res.val = get<Scalar>(lhs.val) - get<Scalar>(rhs.val);
        else if (get_if<Vector>(&lhs.val) and get_if<Vector>(&rhs.val))
            res.val = get<vector<double>>(get<Vector>(lhs.value().val)) - get<vector<double>>(get<Vector>(rhs.value().val));
        else if (get_if<Matrix>(&lhs.val) and get_if<Matrix>(&rhs.val))
            res.val = get<Matrix>(lhs.val) - get<Matrix>(rhs.val);
        else
            throw TError(Error::InvalidOperation);
        return res;
    }
    friend TValue operator * (const TValue &lhs, const TValue &rhs)
    {
        TValue res;

        if (get_if<Scalar>(&lhs.val))
        {
            if (get_if<Scalar>(&rhs.val))
                res.val = get<Scalar>(lhs.val) * get<Scalar>(rhs.val);
            else if (get_if<Vector>(&rhs.val))
                res.val = get<Scalar>(lhs.val) * get<vector<double>>(get<Vector>(rhs.value().val));
            else
                res.val = get<Scalar>(lhs.val) * get<Matrix>(rhs.val);
        }
        else if (get_if<Scalar>(&rhs.val))
        {
            if (get_if<Scalar>(&lhs.val))
                res.val = get<Scalar>(lhs.val) * get<Scalar>(rhs.val);
            else if (get_if<Vector>(&lhs.val))
                res.val = get<vector<double>>(get<Vector>(lhs.value().val)) * get<Scalar>(rhs.val);
            else
                res.val = get<Matrix>(lhs.val) * get<Scalar>(rhs.val);
        }
        else
            throw TError(Error::InvalidOperation);
        return res;
    }
    friend TValue operator / (const TValue &lhs, const TValue &rhs)
    {
        TValue res;

        if (not get_if<Scalar>(&rhs.val))
            throw TError(Error::InvalidOperation);
        if (get_if<Scalar>(&lhs.val))
            res.val = get<Scalar>(lhs.val) / get<Scalar>(rhs.val);
        else if (get_if<Vector>(&lhs.val))
            res.val = get<vector<double>>(get<Vector>(lhs.value().val)) / get<Scalar>(rhs.val);
        else
            res.val = get<Matrix>(lhs.val) / get<Scalar>(rhs.val);
        return res;
    }
};

#endif // VALUE_H
