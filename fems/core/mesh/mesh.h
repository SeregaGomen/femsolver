#ifndef TMESH_H
#define TMESH_H

#include "matrix/matrix.h"
#include "msg/msg.h"

// Типы конечных элементов
enum class FEType { undefined  = 0, fe1d2, fe2d3, fe2d4, fe2d6, fe2d3p, fe2d4p, fe2d6p, fe3d4, fe3d8, fe3d10, fe3d3s, fe3d4s, fe3d6s };


class TMesh
{
private:
    vector<tuple<string, FEType, int, int, int>> fe_type_table {
        { "fe1d2", FEType::fe1d2, 1, 2, 1 },
        { "fe2d3", FEType::fe2d3, 2, 3, 2 },
        { "fe2d4", FEType::fe2d4, 2, 4, 2 },
        { "fe2d6", FEType::fe2d6, 3, 6, 2 },
        { "fe3d4", FEType::fe3d4, 3, 4, 3 },
        { "fe3d8", FEType::fe3d8, 4, 8, 3 },
        { "fe3d10", FEType::fe3d8, 6, 10, 3 },
        { "fe2d3p", FEType::fe2d3p, 0, 3, 2 },
        { "fe2d4p", FEType::fe2d4p, 0, 4, 2 },
        { "fe2d6p", FEType::fe2d6p, 0, 6, 2 },
        { "fe3d3s", FEType::fe3d3s, 0, 3, 3 },
        { "fe3d4s", FEType::fe3d4s, 0, 4, 3 },
        { "fe3d6s", FEType::fe3d6s, 0, 6, 3 },
    };
    FEType type = FEType::undefined;
    vector<vector<int>> mesh_map;
    matrix<double> x;
    matrix<int> fe;
    matrix<int> be;
    FEType decode_mesh_type(string, int&, int&, int&);
    void create_mesh_map(void);
    string fe_name(void);
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
    array<double, 3> get_coord_fe(int, int);
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
    friend ostream &operator << (ostream&, TMesh&);
    void write(ofstream&);
    bool is_1d(void)
    {
        return type == FEType::fe1d2 ? true : false;
    }
    bool is_2d(void)
    {
        return type == FEType::fe2d3 or type == FEType::fe2d4 or type == FEType::fe2d6 ? true : false;
    }
    bool is_3d(void)
    {
        return type == FEType::fe3d4 or type == FEType::fe3d8 or type == FEType::fe3d10 ? true : false;
    }
    bool is_plate(void)
    {
        return type == FEType::fe2d3p or type == FEType::fe2d4p or type == FEType::fe2d6p ? true : false;
    }
    bool is_shell(void)
    {
        return type == FEType::fe3d3s or type == FEType::fe3d4s or type == FEType::fe3d6s ? true : false;
    }
};

#endif // TMESH_H
