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

template <class S> class TFEM
{
private:
    S solver;
    TMesh mesh;
    list<string> program;
    template <typename T> void run(void)
    {
        TParser<T> parser;
        matrix<double> it_m;

        parser.set_program(program);
        solver.setup(&mesh);
        for (auto i = 0u; i < mesh.get_fe().size1(); i++)
        {
            parser.set_data(mesh.get_shape<T>(i));
            it_m = parser.run(mesh.get_coord_fe(i)).asMatrix();

            ansamble_local_matrix(it_m, i);


            cout << it_m << endl;
        }
    }
    void ansamble_local_matrix(matrix<double> &lm, unsigned i)
    {
        unsigned freedom = mesh.get_freedom(),
                 size = (unsigned)lm.size1() * freedom;


        // Учет матрицы
        for (unsigned l = 0; l < size; l++)
        {
            for (unsigned k = l; k < size; k++)
            {
                solver.addMatrix(lm(l, k), mesh.get_fe(i, l / freedom) * freedom + l % freedom, mesh.get_fe(i, k / freedom) * freedom + k % freedom);
                if (l not_eq k)
                    solver.addMatrix(lm(l, k), mesh.get_fe(i, k / freedom) * freedom + k % freedom, mesh.get_fe(i, l / freedom) * freedom + l % freedom);
            }
//            solver.addLoad(fe->getLoad(l), mesh->getFE(i, l / freedom) * freedom + l % freedom);
        }
    }
    string parse_mesh_file_name(string str)
    {
        str = str.substr(str.find_first_not_of(" \t"), str.length());
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

        if (file.is_open())
        {
            while (not file.eof())
            {
                getline(file, str);
                if ((pos = int(str.find("#"))) not_eq -1)
                    mesh.set_mesh_file(filesystem::path(name).parent_path().string(), parse_mesh_file_name(str));
                else
                    program.push_back(str);
            }
            file.close();
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
