#ifndef TFEM_H
#define TFEM_H

#include <numeric>
#include <list>
#include <string>
#include <filesystem>
#include <fstream>
#include "mesh/mesh.h"
#include "shape/shape.h"
#include "parser/parser.h"
#include "shape/shape.h"
#include "analyse/analyse.h"

using namespace std;

//---------------------------------------------------------
// Реализация МКЭ для заданного пользователем функционала
//---------------------------------------------------------
template <class S> class TFEM
{
private:
    // Имя файла с программой расчета
    string prog_name;
    // Решатель СЛАУ
    S solver;
    // Сетка
    TMesh mesh;
    // Функционал и разрешающие соотношения
    list<string> program;
    // Результаты расчета
    TResultList results;
    // Запуск вычислительного процесса
    template <typename T> void run(void)
    {
        TParser<T> parser;
        vector<double> res;

        parser.set_program(program);
        solver.setup(mesh);
        create_global_matrix(parser);
        use_boundary_condition(parser);
        if (solve_equations(res))
        {
            calc_results(parser, res);
            save_result(prog_name.substr(0, prog_name.find_last_of(".")) + ".res");
            print_result_summary();
        }
    }
    // Формирование глобальной матрицы жесткости
    template <typename T> void create_global_matrix(TParser<T> &parser)
    {
        TProgress progress;

        progress.set_process(Message::GeneratingMatrix, 1, (int)mesh.get_fe().size1());
        for (auto i = 0u; i < mesh.get_fe().size1(); i++)
        {
            progress.add_progress();
            parser.set_data(mesh.get_shape<T>(i));
            ansamble_local_matrix(parser.run(mesh.get_coord_fe(i)).asMatrix(), i);
        }
        progress.stop_process();
    }
    // Учет граничных условий
    template <typename T> void use_boundary_condition(TParser<T> &parser)
    {
        TProgress progress;
        list<tuple<int, int, int, double>> bc;

        progress.set_process(Message::UsingBoundaryCondition);
        parser.get_boundary_conditions(mesh, bc);
        for (auto [i, type, dir, val]: bc)
            (type == 1) ? solver.setBoundaryCondition(i * mesh.get_freedom() + dir, val) : solver.setLoad(i * mesh.get_freedom() + dir, val);
        progress.stop();
    }
    // Решение СЛАУ
    bool solve_equations(vector<double> &res)
    {
        double eps = 1.0E-10;
        bool is_aborted = false,
             ret;

        ///////////////////////////
        // solver.print("matrix.res");
        ///////////////////////////

        ret = solver.solve(res, eps, is_aborted);
//        if (!is_aborted and ret)
//            cout << res;
        return (is_aborted) ? false : ret;
    }
    // Вычисление деформаций и напряжений
    template <typename T> void calc_results(TParser<T> &parser, vector<double> &u)
    {
        TProgress progress;
        matrix<double> res(parser.get_result_table().size() + parser.get_function_table().size(), mesh.get_x().size1());
        vector<double> fe_u,
                       value,
                       counter(mesh.get_x().size1()); // Счетчик кол-ва вхождения узлов для осреднения результатов

        // Копируем результаты расчета (перемещения)
        for (auto i = 0u; i < mesh.get_x().size1(); i++)
            for (auto j = 0; j < mesh.get_freedom(); j++)
                res(j, i) = u[i * mesh.get_freedom() + j];
        // Вычисляем вспомогательные функции (деформации и напряжения)
        progress.set_process(Message::GeneratingResult, 1, (int)mesh.get_fe().size1());
        for (auto i = 0; i < mesh.get_fe().size1(); i++)
        {
            progress.add_progress();
            // Формируем вектор перемещений для текущего КЭ
            fe_u.resize(mesh.get_fe().size2() * mesh.get_freedom());
            for (auto j = 0u; j < mesh.get_fe().size2(); j++)
                for (auto k = 0; k < mesh.get_freedom(); k++)
                    fe_u[j * mesh.get_freedom() + k] = u[mesh.get_freedom() * mesh.get_fe(i, j) + k];
            // Загружаем результирующие функции (перемещения)
            parser.set_data(mesh.get_shape<T>(i), fe_u);
            for (auto j = 0u; j < parser.get_function_table().size(); j++)
                for (auto k = 0u; k < mesh.get_fe().size2(); k++)
                {
                    TValue<T>::x = mesh.get_coord_fe(i, k);
                    value = parser.get_function_table()[j].second.value().asVector();
                    res(mesh.get_freedom() + j, mesh.get_fe(i, k)) += accumulate(value.begin(), value.end(), 0.0);
                    if (j == 0)
                        counter[mesh.get_fe(i, k)]++;
                }
        }
        progress.stop_process();
        // Осредняем результаты
        for (auto i = mesh.get_freedom(); i < (int)res.size1(); i++)
            for (auto j = 0u; j< mesh.get_x().size1(); j++)
                res[i][j] /= counter[j];
        // Cохраняем результаты
        for (auto i = 0u; i < res.size1(); i++)
            results.set_result(res[i], (int)res.size2(), i < parser.get_result_table().size() ? parser.get_result_table()[i].first : parser.get_function_table()[i - mesh.get_freedom()].first);
    }
    // Ансамблирование локальной матрицы жесткости к глобальной
    void ansamble_local_matrix(const matrix<double> &lm, unsigned i)
    {
        unsigned freedom = mesh.get_freedom(),
                 size = (unsigned)lm.size1();

        /////////////////
        // cout << lm << endl;
        /////////////////
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
            throw TError(Message::Preprocessor);
        str = str.substr(1, str.length());
        str = str.substr(str.find_first_not_of(" \t"), str.length());
        if (str.substr(0, 4) not_eq "mesh" )
            throw TError(Message::Preprocessor);
        str = str.substr(5, str.length());
        return str.substr(str.find_first_not_of(" \t"), str.length());
    }
public:
    TFEM(void) noexcept {}
    ~TFEM(void) noexcept = default;
    void set_program(string name)
    {
        fstream file(prog_name = name);
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
                throw TError(Message::NotMesh);
        }
        else
            throw TError(Message::ReadFile);
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
        case FEType::fe2d4:
            run<TShape<TShape2d4>>();
            break;
        case FEType::fe3d4:
            run<TShape<TShape3d4>>();
            break;
        default:
            throw TError(Message::IncorrectFE);
        }
    }
    void save_result(string name)
    {
        ofstream out;
        TProgress progress;

        out.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            out.open(name);
            out.precision(16);

            progress.set_process(Message::WritingResult);
            // Запись подписи
            out << "FEM Solver Results File" << endl;
            // Запись сетки
            mesh.write(out);
            // Запись результатов
            results.write(out);
            out.close();
            progress.stop();
        }
        catch (fstream::failure&)
        {
            progress.stop();
            throw TError(Message::ReadFile);
        }
    }
    // Вывод рез-тов по каждой функции на экран
    void print_result_summary(double t = 0)
    {
        int width = 15,
            precision = 8;

        cout << "---------------------------------------------------------------------" << endl;
        cout.setf(ios::left);
        cout << setw(10) << ' ' << setw(width) << "\tmin" << ' ' << setw(width) << "\tmax" << endl;
        cout.flags (ios::floatfield | ios::scientific | ios::showpos);
        for (unsigned i = 0; i < results.size(); i++)
            if (results[i].get_time() == t)
                cout << setw(10) << results[i].get_name() << '\t' << std::scientific << setw(width) <<
                        setprecision(precision) << results[i].min() << '\t' << setw(width) << results[i].max() << endl;
        cout.unsetf(ios::showpos);
        cout << "---------------------------------------------------------------------" << endl;
    }
};

#endif // TFEM_H
