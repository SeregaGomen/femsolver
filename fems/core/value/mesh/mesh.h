#ifndef TMESH_H
#define TMESH_H

#include "matrix/matrix.h"

// Типы конечных элементов
enum class FEType { undefined  = 0, fe1d2, fe2d3, fe2d4, fe2d6, fe2d3p, fe2d4p, fe2d6p, fe3d4, fe3d8, fe3d10, fe3d3s, fe3d4s, fe3d6s };


class TMesh
{
private:
    FEType type = FEType::undefined;
    matrix<double> x;
    matrix<int> fe;
    matrix<int> be;
    FEType decode_mesh_type(int, int&, int&, int&);
public:
    TMesh(void) noexcept {}
    ~TMesh(void) noexcept = default;
    FEType get_type(void) const noexcept
    {
        return type;
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
    void set_mesh_file(string);
};

#endif // TMESH_H
