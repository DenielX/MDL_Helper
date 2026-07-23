#ifndef MODEL_SIMPLE_H
#define MODEL_SIMPLE_H

#include "model_info.h"

class ModelSimple : public ModelInfo
{
public:
    ModelSimple(FILE* file, unsigned short cnt);
    ~ModelSimple() override;

    unsigned int* GetAtomicOfst() override;
    unsigned int* GetGeometryOfst() override;
    GeometryHeader* GetGeometryHeader() override;
    unsigned int** GetMaterialOfst() override;
    std::string** GetMaterialList() override;
    SubMeshHeader** GetSubMeshHeaders() override;
};

#endif