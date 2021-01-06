#ifndef TMESH_H
#define TMESH_H

#include "matrix/matrix.h"
#include "msg/msg.h"

// Типы конечных элементов
enum class FEType { undefined  = 0, fe1d2, fe2d3, fe2d4, fe2d6, fe2d3p, fe2d4p, fe2d6p, fe3d4, fe3d8, fe3d10, fe3d3s, fe3d4s, fe3d6s };


class TMesh
{
private:
    FEType type = FEType::undefined;
    vector<vector<int>> mesh_map;
    matrix<double> x;
    matrix<int> fe;
    matrix<int> be;
    FEType decode_mesh_type(int, int&, int&, int&);
    void create_mesh_map(void);
public:
    TMesh(void) noexcept {}
    ~TMesh(void) noexcept = default;
    FEType get_type(void) const noexcept
    {
        return type;
    }
    matrix<double> &get_x(void) noexcept
    {
        return x;
    }
    matrix<int> &get_fe(void) noexcept
    {
        return fe;
    }
    matrix<int> &get_be(void) noexcept
    {
        return be;
    }
    double get_x(int i, int j) const noexcept
    {
        return x(i, j);
    }
    int get_fe(int i, int j) const noexcept
    {
        return fe(i, j);
    }
    int get_be(int i, int j) const noexcept
    {
        return be(i, j);
    }
    int get_freedom(void);
    vector<int>& get_mesh_map(int i)
    {
        return mesh_map[i];
    }
    void set_mesh_file(string, string);
    matrix<double> get_coord_fe(int);
    template <class T> vector<T> get_shape(int i)
    {
        matrix<double> px = get_coord_fe(i),
                       m(T::size(), T::size() + 1);
        vector<double> v(T::size());
        vector<T> res;

        for (auto i = 0; i < T::size(); i++)
        {
            for (auto j = 0; j < T::size(); j++)
            {
                for (auto k = 0; k < T::size(); k++)
                    m(j, k) = T::coeff(px, j, k);
                m(j, T::size()) = (i == j) ? 1.0 : 0.0;
            }
            if (not solve(m, v))
                throw TError(Message::InvalidFE);
            res.push_back(T(v));
        }
        return res;
    }
};


//void TShape::create(matrix<double>& px)
//{
//    using namespace Eigen;

//    MatrixXd A(size, size);
//    VectorXd b(size);

//    x = px;
//    c.resize(size, size);
//    for (unsigned i = 0; i < size; i++)
//    {
//        for (unsigned j = 0; j < size; j++)
//        {
//            for (unsigned k = 0; k < size; k++)
//                A(j, k) = shape_coeff(j, k);
//            b(j) = (i == j) ? 1.0 : 0.0;
//        }
//        if (A.determinant() == 0.0)
//            throw ErrorCode::EIncorrectFE;
//        b = A.colPivHouseholderQr().solve(b);
//        for (unsigned j = 0; j < size; j++)
//            c(j, i) = b(j);
//    }
//}
//--------------------------------------------------------------

#endif // TMESH_H
