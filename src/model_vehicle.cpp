#include "model_vehicle.h"

ModelVehicle::ModelVehicle(FILE* file, unsigned short cnt) : ModelInfo(file, cnt)
{
    this->type_mdl = TypeMDL::Vehicle;

    // порядок вызова ИМЕЕТ значение:
    this->GetClumpOfst();             //получаем указатель на кламп (1 кламп у vehicle, остальное через extra цепляется, rslElementGroup = 0x02)
    //this->GetAtomicOfst();          //получаем указатель на атомик(rslElement = 0x01)
}

ModelVehicle::~ModelVehicle()
{}//пока пустой

unsigned int* ModelVehicle::GetClumpOfst()  // Доделано.
{
    this->psp_clump_ofst.reset(new unsigned int);

    fseek(mdl_file, 0x20, SEEK_SET);                                     //переходим на *clump_ptr
    fread(this->psp_clump_ofst.get(), sizeof(unsigned int), 1, mdl_file);//запоминаем его

    return psp_clump_ofst.get();
}

unsigned int* ModelVehicle::GetAtomicOfst()
{
    this->psp_atomic_ofst.reset(new unsigned int[this->cnt_of_atomics]);

    fseek(mdl_file, 0x20, SEEK_SET);                                         //переходим на *atomic_ptr
    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        fread(&this->psp_atomic_ofst[n], sizeof(unsigned int*), 1, mdl_file);//запоминаем их
    }

    return psp_atomic_ofst.get();
}

// Заглушки, пока парсинг транспорта не готов:
unsigned int* ModelVehicle::GetGeometryOfst() {
    return nullptr;
}

ModelInfo::GeometryHeader* ModelVehicle::GetGeometryHeader() {
    return nullptr;
}

unsigned int** ModelVehicle::GetMaterialOfst() {
    return nullptr;
}

std::string** ModelVehicle::GetMaterialList() {
    return nullptr;
}

ModelInfo::SubMeshHeader** ModelVehicle::GetSubMeshHeaders() {
    return nullptr;
}