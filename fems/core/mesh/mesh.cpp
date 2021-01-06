#include <filesystem>
#include <fstream>
#include "mesh.h"
#include "msg/msg.h"
#include "shape/shape.h"

// ------------- Определение параметров КЭ ----------------------
FEType TMesh::decode_mesh_type(string type, int& be_size, int& fe_size, int& dim)
{
    FEType ret = FEType::undefined;
    auto index = find_if(fe_type_table.begin(), fe_type_table.end(), [type](const auto &it) { return get<0>(it) == type; });

    if (index != fe_type_table.end())
    {
        ret = get<1>(*index);
        be_size = get<2>(*index);
        fe_size = get<3>(*index);
        dim = get<4>(*index);
    }
    return ret;
}

void TMesh::set_mesh_file(string path, string name)
{
    string fetype;
    int val,
        fe_size,
        be_size,
        dim;
    ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        if (filesystem::exists(name))
            file.open(name);
        else
            file.open(path + "/" + name);
        file >> fetype;
        if ((type = decode_mesh_type(fetype, be_size, fe_size, dim)) == FEType::undefined)
            throw TError(Message::MeshFormat);
        file >> val;
        if (val <= 0 or dim < 1 or dim > 3)
            throw TError(Message::MeshFormat);
        x.resize(val, dim);
        for (auto i = 0; i < val; i++)
            for (auto j = 0; j < dim; j++)
                file >> x(i, j);
        file >> val;
        if (val == 0)
            throw TError(Message::MeshFormat);
        fe.resize(val, fe_size);
        for (auto i = 0; i < val; i++)
            for (auto j = 0; j < fe_size; j++)
                file >> fe(i, j);
        file >> val;
        if (val == 0 and (type == FEType::fe2d3 or type == FEType::fe2d4 or type == FEType::fe3d4 or type == FEType::fe3d8))
            throw TError(Message::MeshFormat);
        if ((type == FEType::fe2d3p or type == FEType::fe2d4p or type == FEType::fe2d6) or (type == FEType::fe3d3s or type == FEType::fe3d4s or type == FEType::fe3d6s))
            be = fe;
        else // if (feDim not_eq 1)
        {
            be.resize(val, be_size);
            for (auto i = 0; i < val; i++)
                for (auto j = 0; j < be_size; j++)
                    file >> be(i, j);
        }
        file.close();
        cout << *this << endl;
        create_mesh_map();
    }
    catch (fstream::failure&)
    {
        file.close();
        throw TError(Message::ReadFile);
    }
}

matrix<double> TMesh::get_coord_fe(int index)
{
    matrix<double> coord(fe.size2(), 3);

    for (auto i = 0u; i < fe.size2(); i++)
        for (auto j = 0u; j < x.size2(); j++)
            coord(i, j) = x(fe(index, i), j);
    return coord;
}

int TMesh::get_freedom(void)
{
    int ret = 0;

    switch (type)
    {
    case FEType::fe1d2:
        ret = 1;
        break;
    case FEType::fe2d3:
    case FEType::fe2d4:
    case FEType::fe2d6:
        ret = 2;
        break;
    case FEType::fe2d3p:
    case FEType::fe2d4p:
    case FEType::fe2d6p:
    case FEType::fe3d4:
    case FEType::fe3d8:
    case FEType::fe3d10:
        ret = 3;
        break;
    case FEType::fe3d3s:
    case FEType::fe3d4s:
    case FEType::fe3d6s:
        ret = 6;
        break;
    default:
        ret = 0;
    }
    return ret;
}

void TMesh::create_mesh_map(void)
{
    TProgress progress;

    mesh_map.resize(x.size1());
    progress.set_process(Message::AnalysingMesh, 1, int(fe.size1()));
    for (unsigned i = 0; i < fe.size1(); /*msg->addProgress(),*/ i++)
        for (unsigned j = 0; j < fe.size2(); j++)
            for (unsigned k = 0; k < fe.size2(); k++)
                if (k not_eq j)
                    if (find(mesh_map[fe(i, j)].begin(), mesh_map[fe(i, j)].end(), fe(i, k)) == mesh_map[fe(i, j)].end())
                        mesh_map[fe(i, j)].push_back(fe(i, k));

    for (unsigned i = 0; i < mesh_map.size(); i++)
        sort(mesh_map[i].begin(), mesh_map[i].end(), [](unsigned k, unsigned l) -> bool{ return (k < l); });
    progress.stop_process();
}

string TMesh::fe_name(void)
{
    vector<pair<FEType, Message>> table{ { FEType::fe1d2, Message::FE1D2 }, { FEType::fe2d3, Message::FE2D3 }, { FEType::fe2d4, Message::FE2D4 },
                                         { FEType::fe2d6, Message::FE2D6 }, { FEType::fe3d4, Message::FE3D4 }, { FEType::fe3d8, Message::FE3D8 },
                                         { FEType::fe3d10, Message::FE3D10 }, { FEType::fe2d3p, Message::FE2D3P }, { FEType::fe2d4p, Message::FE2D4P },
                                         { FEType::fe2d6p, Message::FE2D6P }, { FEType::fe3d3s, Message::FE3D3S }, { FEType::fe3d4s, Message::FE3D4S },
                                         { FEType::fe3d6s, Message::FE3D6S } };

    return say_message(find_if(table.begin(), table.end(), [this](pair<FEType, Message> i) { return i.first == this->type; } )->second);
}

ostream &operator << (ostream &out, TMesh &r)
{
    out << say_message(Message::FEType) << r.fe_name() << endl;
    out << say_message(Message::NumNodes) << r.x.size1() << endl;
    out << say_message(Message::NumFE) << r.fe.size1() << endl;
    return out;
}

void TMesh::write(ofstream &out)
{
    out << "Mesh" << endl;
    out << get<0>(*find_if(fe_type_table.begin(), fe_type_table.end(), [this](const auto &it) { return get<1>(it) == this->type; }))  << endl;
    out << x.size1() << endl;
    for (auto i = 0u; i < x.size1(); i++)
    {
        for (auto j = 0u; j < x.size2(); j++)
            out << x(i, j) << ' ';
        out << endl;
    }
    out << fe.size1() << endl;
    for (auto i = 0u; i < fe.size1(); i++)
    {
        for (auto j = 0u; j < fe.size2(); j++)
            out << fe(i, j) << ' ';
        out << endl;
    }
    if (is_plate() or is_shell())
        out << 0 << endl;
    else
    {
        out << be.size1() << endl;
        for (unsigned i = 0; i < be.size1(); i++)
        {
            for (unsigned j = 0; j < be.size2(); j++)
                out << be(i, j) << ' ';
            out << endl;
        }
    }
}
