#ifndef TFEM_H
#define TFEM_H

#include <list>
#include <string>
#include <filesystem>
#include <fstream>
#include "mesh/mesh.h"
#include "shape/shape.h"
#include "parser/parser.h"
#include "shape/shape.h"

using namespace std;

//---------------------------------------------------------
// Реализация МКЭ для заданного пользователем функционала
//---------------------------------------------------------
template <class S> class TFEM
{
private:
    // Решатель СЛАУ
    S solver;
    // Сетка
    TMesh mesh;
    // Функционал и разрешающие соотношения
    list<string> program;
    // Запуск вычислительного процесса
    template <typename T> void run(void)
    {
        TParser<T> parser;

        parser.set_program(program);
        solver.setup(mesh);
        create_global_matrix(parser);
        use_boundary_condition(parser);
        solve_equations();
    }
    // Формирование глобальной матрицы жесткости
    template <typename T> void create_global_matrix(TParser<T> &parser)
    {
        matrix<double> it_m;

        for (auto i = 0u; i < mesh.get_fe().size1(); i++)
        {
            parser.set_data(mesh.get_shape<T>(i));
            it_m = parser.run(mesh.get_coord_fe(i)).asMatrix();
            ansamble_local_matrix(it_m, i);


            cout << it_m << endl;
        }
    }
    // Учет граничных условий
    template <typename T> void use_boundary_condition(TParser<T> &parser)
    {
        list<tuple<int, int, int, double>> bc;

        parser.get_boundary_conditions(mesh, bc);
        for (auto [i, type, dir, val]: bc)
            (type == 1) ? solver.setBoundaryCondition(i * mesh.get_freedom() + dir, val) : solver.setLoad(i * mesh.get_freedom() + dir, val);

    }
    // Решение СЛАУ
    bool solve_equations(void)
    {
        double eps = 1.0E-10;
        bool is_aborted = false,
             ret;
        vector<double> res;

        //solver.print("matrix.res");
        ret = solver.solve(res, eps, is_aborted);
        if (!is_aborted and ret)
            cout << res;
        return (is_aborted) ? false : ret;
    }
    // Ансамблирование локальной матрицы жесткости к глобальной
    void ansamble_local_matrix(matrix<double> &lm, unsigned i)
    {
        unsigned freedom = mesh.get_freedom(),
                 size = (unsigned)lm.size1();


        // Учет матрицы
        for (unsigned l = 0; l < size; l++)
        {
            for (unsigned k = l; k < size; k++)
            {
                solver.addMatrix(lm(l, k), mesh.get_fe(i, l / freedom) * freedom + l % freedom, mesh.get_fe(i, k / freedom) * freedom + k % freedom);
                if (l not_eq k)
                    solver.addMatrix(lm(l, k), mesh.get_fe(i, k / freedom) * freedom + k % freedom, mesh.get_fe(i, l / freedom) * freedom + l % freedom);
            }
            solver.addLoad(lm(l, size), mesh.get_fe(i, l / freedom) * freedom + l % freedom);
        }
    }
    string parse_mesh_file_name(string str)
    {
        if (str[0] not_eq '#')
            throw TError(Error::Preprocessor);
        str = str.substr(1, str.length());
        str = str.substr(str.find_first_not_of(" \t"), str.length());
        if (str.substr(0, 4) not_eq "mesh" )
            throw TError(Error::Preprocessor);
        str = str.substr(5, str.length());
        return str.substr(str.find_first_not_of(" \t"), str.length());
    }
public:
    TFEM(void) noexcept {}
    ~TFEM(void) noexcept = default;
    void set_program(string name)
    {
        fstream file(name);
        string str;
        int pos;
        bool is_mesh = false;

        if (file.is_open())
        {
            while (not file.eof())
            {
                getline(file, str);
                if (not str.length())
                    continue;
                str = str.substr(str.find_first_not_of(" \t"), str.length());
                if (str[0] == '/' and str[1] == '/')
                    continue;
                if ((pos = int(str.find("#"))) not_eq -1)
                {
                    mesh.set_mesh_file(filesystem::path(name).parent_path().string(), parse_mesh_file_name(str));
                    is_mesh = true;
                }
                else
                    program.push_back(str);
            }
            file.close();
            if (not is_mesh)
                throw TError(Error::NotMesh);
        }
        else
            throw TError(Error::ReadFile);
    }
    void start(void)
    {
        switch (mesh.get_type())
        {
        case FEType::fe1d2:
            run<TShape<TShape1d2>>();
            break;
        case FEType::fe2d3:
            run<TShape<TShape2d3>>();
            break;
        default:
            throw TError(Error::IncorrectFE);
        }
    }
};

#endif // TFEM_H
