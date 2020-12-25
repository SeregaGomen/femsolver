#include <filesystem>
#include <fstream>
#include "mesh.h"
#include "error/error.h"
#include "shape/shape.h"

// ------------- Определение параметров КЭ ----------------------
FEType TMesh::decode_mesh_type(int type, int& fe_size, int& be_size, int& dim)
{
    FEType ret;

    switch (type)
    {
        case 3:
            ret = FEType::fe2d3;
            be_size = 2;
            fe_size = 3;
            dim = 2;
            break;
        case 4:
            ret = FEType::fe3d4;
            be_size = 3;
            fe_size = 4;
            dim = 3;
            break;
        case 6:
            ret = FEType::fe2d6;
            be_size = 3;
            fe_size = 6;
            dim = 2;
            break;
        case 8:
            ret = FEType::fe3d8;
            be_size = 4;
            fe_size = 8;
            dim = 3;
            break;
        case 10:
            ret = FEType::fe3d10;
            be_size = 6;
            fe_size = 10;
            dim = 3;
            break;
        case 24:
            ret = FEType::fe2d4;
            be_size = 2;
            fe_size = 4;
            dim = 2;
            break;
        case 34:
            ret = FEType::fe1d2;
            be_size = 1;
            fe_size = 2;
            dim = 1;
            break;
        case 123:
            ret = FEType::fe2d3p;
            be_size = 0;
            fe_size = 3;
            dim = 2;
            break;
        case 124:
            ret = FEType::fe2d4p;
            be_size = 0;
            fe_size = 4;
            dim = 2;
            break;
        case 125:
            ret = FEType::fe2d6p;
            be_size = 0;
            fe_size = 6;
            dim = 2;
            break;
        case 223:
            ret = FEType::fe3d3s;
            be_size = 0;
            fe_size = 3;
            dim = 3;
            break;
        case 224:
            ret = FEType::fe3d4s;
            be_size = 0;
            fe_size = 4;
            dim = 3;
            break;
        case 225:
            ret = FEType::fe3d6s;
            be_size = 0;
            fe_size = 6;
            dim = 3;
            break;
        default:
            fe_size = be_size = dim = 0;
            ret = FEType::undefined;
    }
    return ret;
}

void TMesh::set_mesh_file(string path, string name)
{
    ifstream file;
    int val,
        fe_size,
        be_size,
        dim;

    file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        if (filesystem::exists(name))
            file.open(name);
        else
            file.open(path + "/" + name);
        file >> val;
        if ((type = decode_mesh_type(val, fe_size, be_size, dim)) == FEType::undefined)
        {
            file.close();
            throw TError(Error::MeshFormat);
        }
        file >> val;
        if (val <= 0 or dim < 1 or dim > 3)
        {
            file.close();
            throw TError(Error::MeshFormat);
        }
        x.resize(val, dim);
        for (auto i = 0; i < val; i++)
            for (auto j = 0; j < dim; j++)
                file >> x(i, j);
        file >> val;
        if (val == 0)
        {
            file.close();
            throw TError(Error::MeshFormat);
        }
        fe.resize(val, fe_size);
        for (auto i = 0; i < val; i++)
            for (auto j = 0; j < fe_size; j++)
                file >> fe(i, j);
        file >> val;
        if (val == 0 and (type == FEType::fe2d3 or type == FEType::fe2d4 or type == FEType::fe3d4 or type == FEType::fe3d8))
        {
            file.close();
            throw TError(Error::MeshFormat);
        }
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
        create_mesh_map();
    }
    catch (fstream::failure&)
    {
        throw TError(Error::ReadFile);
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
    mesh_map.resize(x.size1());
//    msg->setProcess(ProcessCode::AnalysingMesh, 1, int(fe.size1()));
    for (unsigned i = 0; i < fe.size1(); /*msg->addProgress(),*/ i++)
        for (unsigned j = 0; j < fe.size2(); j++)
            for (unsigned k = 0; k < fe.size2(); k++)
                if (k not_eq j)
                    if (find(mesh_map[fe(i, j)].begin(), mesh_map[fe(i, j)].end(), fe(i, k)) == mesh_map[fe(i, j)].end())
                        mesh_map[fe(i, j)].push_back(fe(i, k));

    for (unsigned i = 0; i < mesh_map.size(); i++)
        sort(mesh_map[i].begin(), mesh_map[i].end(), [](unsigned k, unsigned l) -> bool{ return (k < l); });
//    msg->stopProcess();
}
