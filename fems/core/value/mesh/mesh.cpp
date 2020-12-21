#include <fstream>
#include "mesh.h"

// ------------- Определение параметров КЭ ----------------------
FEType TMesh::decode_mesh_type(int type, int& feSize, int& beSize, int& feDim)
{
    FEType ret;

    switch (type)
    {
        case 3:
            ret = FEType::fe2d3;
            beSize = 2;
            feSize = 3;
            feDim = 2;
            break;
        case 4:
            ret = FEType::fe3d4;
            beSize = 3;
            feSize = 4;
            feDim = 3;
            break;
        case 6:
            ret = FEType::fe2d6;
            beSize = 3;
            feSize = 6;
            feDim = 2;
            break;
        case 8:
            ret = FEType::fe3d8;
            beSize = 4;
            feSize = 8;
            feDim = 3;
            break;
        case 10:
            ret = FEType::fe3d10;
            beSize = 6;
            feSize = 10;
            feDim = 3;
            break;
        case 24:
            ret = FEType::fe2d4;
            beSize = 2;
            feSize = 4;
            feDim = 2;
            break;
        case 34:
            ret = FEType::fe1d2;
            beSize = 1;
            feSize = 2;
            feDim = 1;
            break;
        case 123:
            ret = FEType::fe2d3p;
            beSize = 0;
            feSize = 3;
            feDim = 2;
            break;
        case 124:
            ret = FEType::fe2d4p;
            beSize = 0;
            feSize = 4;
            feDim = 2;
            break;
        case 125:
            ret = FEType::fe2d6p;
            beSize = 0;
            feSize = 6;
            feDim = 2;
            break;
        case 223:
            ret = FEType::fe3d3s;
            beSize = 0;
            feSize = 3;
            feDim = 3;
            break;
        case 224:
            ret = FEType::fe3d4s;
            beSize = 0;
            feSize = 4;
            feDim = 3;
            break;
        case 225:
            ret = FEType::fe3d6s;
            beSize = 0;
            feSize = 6;
            feDim = 3;
            break;
        default:
            feSize = beSize = feDim = 0;
            ret = FEType::undefined;
    }
    return ret;
}

void TMesh::setMeshFile(string name)
{
    ifstream file;
    int val;

    file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    file.open(name);
    file >> val;
    if ((feType = getDataFE(temp, feSize, surfaceSize, feDim)) == FEType::undefined)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    in >> temp;
    if (in.fail() or temp == 0 or feDim < 1 or feDim > 3)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    x.resize(temp, feDim);

    for (unsigned i = 0; i < temp; i++)
    {
        for (unsigned j = 0; j < feDim; j++)
            in >> x(i, j);
        if (in.fail())
        {
            in.close();
            cerr << sayError(ErrorCode::EReadFile) << endl;
            return (error = true);
        }
    }
    in >> temp;
    if (in.fail() or temp == 0)
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    fe.resize(temp, feSize);
    for (unsigned i = 0; i < temp; i++)
        for (unsigned j = 0; j < feSize; j++)
        {
            in >> fe(i, j);
            if (in.fail())
            {
                in.close();
                cerr << sayError(ErrorCode::EReadFile) << endl;
                return (error = true);
            }
        }
    in >> temp;
    if (in.fail() or (temp == 0 and (feType == FEType::fe2d3 or feType == FEType::fe2d4 or feType == FEType::fe3d4 or feType == FEType::fe3d8)))
    {
        in.close();
        cerr << sayError(ErrorCode::EFormatFile) << endl;
        return (error = true);
    }
    if (isPlate() or isShell())
        be = fe;
    else // if (feDim not_eq 1)
    {
        be.resize(temp, surfaceSize);
        for (unsigned i = 0; i < temp; i++)
            for (unsigned j = 0; j < surfaceSize; j++)
            {
                in >> be(i, j);
                if (in.fail())
                {
                    in.close();
                    cerr << sayError(ErrorCode::EReadFile) << endl;
                    return (error = true);
                }
            }
    }
    in.close();


}
