#ifndef MODEL_VEHICLE_H
#define MODEL_VEHICLE_H

#include "model_info.h"

class ModelVehicle : public ModelInfo
{
public:
    ModelVehicle(FILE* file, unsigned short cnt);
    ~ModelVehicle() override;

    unsigned int* GetClumpOfst() override;
    unsigned int* GetAtomicOfst() override;
    unsigned int* GetGeometryOfst() override;
    GeometryHeader* GetGeometryHeader() override;
    unsigned int** GetMaterialOfst() override;
    std::string** GetMaterialList() override;
    SubMeshHeader** GetSubMeshHeaders() override;
};

#endif