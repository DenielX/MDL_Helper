#include "model_simple.h"

///FILE* debug;

ModelSimple::ModelSimple(FILE* file, unsigned short cnt) : ModelInfo(file, cnt)
{
    this->type_mdl = TypeMDL::SimpleModel;

    // порядок вызова ИМЕЕТ значение:
    this->GetAtomicOfst();                          //получаем указатель на атомик(rslElement = 0x01)
    this->GetGeometryOfst();                        //на его основании, получаем указатель на геометрию(rslGeometry = 0x08)
    this->GetGeometryHeader();                      //по сути получаем адрес Clump->Struct и структуру
    this->GetMaterialOfst();                        //вычисляем адрес списка материалов по цепочке указателей, стартующей в rslGeometry
    this->GetMaterialList();                        //читаем строки материалов по данному указателю
    this->GetSubMeshHeaders();                      //находим хедеры сабмешей (aka sPspGeometryMesh в librw)
}

ModelSimple::~ModelSimple()
{
    if (this->cnt_of_atomics > 0 && this->psp_material_list_ofst) {
        for (unsigned short n = 0; n < this->cnt_of_atomics; ++n) {
            delete[] this->psp_material_list_ofst[n];     // Удаляем каждый указатель на названия текстур
            delete[] this->sub_mesh_header_list[n];       // Удаляем каждый указатель на SubMeshHeader
            delete[] this->psp_material_string_array[n];  // Удаляем список указателей конкретного Atomic
        }
    }
    // Внешние массивы - забота std::unique_ptr
}

unsigned int* ModelSimple::GetAtomicOfst()
{
    this->psp_atomic_ofst.reset(new unsigned int[this->cnt_of_atomics]);

    fseek(mdl_file, 0x20, SEEK_SET);                                        //переходим на *atomic_ptr
    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        fread(&this->psp_atomic_ofst[n], sizeof(unsigned int*), 1, mdl_file);//запоминаем его
    }

    return psp_atomic_ofst.get();
}

unsigned int* ModelSimple::GetGeometryOfst()
{
    this->psp_geometry_ofst.reset(new unsigned int[this->cnt_of_atomics]);

    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        psp_geometry_ofst[n] = psp_atomic_ofst[n] + 20;                        //находим указатель на секцию геометрии

        fseek(mdl_file, psp_geometry_ofst[n], SEEK_SET);                       //и переходим на него
        fread(&this->psp_geometry_ofst[n], sizeof(unsigned int*), 1, mdl_file);//запоминаем его
    }

    return psp_geometry_ofst.get();
}

ModelInfo::GeometryHeader* ModelSimple::GetGeometryHeader()
{
    this->header.reset(new GeometryHeader[this->cnt_of_atomics]);            //выделяем память на структуру хедеров геометрии атомиков
    this->geometry_header_ofst.reset(new unsigned int[this->cnt_of_atomics]);//и на указатели на хедеры

    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        geometry_header_ofst[n] = psp_geometry_ofst[n] + 32;

        fseek(mdl_file, geometry_header_ofst[n], SEEK_SET);                 //переходим на начало структуры
        fread(&header[n], sizeof(GeometryHeader), 1, mdl_file);             //пробуем прочесть сразу всю
        //косвенный указатель на меш стал прямым.Делил на 4,т.к. тут арифметика указателей:
        header[n].psp_geometry_mesh_ofst = header[n].psp_geometry_mesh_ofst + geometry_header_ofst[n] / sizeof(unsigned int*);
    }

    this->GetFlags(&header[0].psp_flags);

    return header.get();
}

unsigned int** ModelSimple::GetMaterialOfst()
{
    //unsigned int tempMatPtr[this->CntOfAtomics];//в Code:Blocks работает
    std::unique_ptr<unsigned int[]> temp_mat_ofst(new unsigned int[this->cnt_of_atomics]);
    this->num_of_materials.reset(new unsigned int[this->cnt_of_atomics]);
    this->psp_material_list_ofst.reset(new unsigned int* [this->cnt_of_atomics]);

    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        fseek(mdl_file, psp_geometry_ofst[n] + 16, SEEK_SET);                           //переходим на адрес кол-ва текстур
        fread(&this->num_of_materials[n], sizeof(unsigned int*), 1, mdl_file);          //запоминаем их кол-во

        this->psp_material_list_ofst[n] = new unsigned int[this->num_of_materials[n]];  //выделяем память именно для цепочки материалов этого Atomic

        this->psp_material_list_ofst[n][0] = psp_geometry_ofst[n] + 12;                 //вычисляем отправной указатель поиска материалов

        fseek(mdl_file, this->psp_material_list_ofst[n][0], SEEK_SET);
        fread(&temp_mat_ofst[n], sizeof(unsigned int), 1, mdl_file);

        //переходим на вереницу указателей на секцию материалов:
        for (unsigned int i = 0; i < this->num_of_materials[n]; ++i) {
            fseek(mdl_file, temp_mat_ofst[n], SEEK_SET);
            fread(&psp_material_list_ofst[n][i], sizeof(unsigned int*), 1, mdl_file);

            fseek(mdl_file, psp_material_list_ofst[n][i], SEEK_SET);
            fread(&psp_material_list_ofst[n][i], sizeof(unsigned int*), 1, mdl_file);

            temp_mat_ofst[n] += 4;
        }
    }

    return psp_material_list_ofst.get();
}

std::string** ModelSimple::GetMaterialList()//TODO: упростить MaterialList до одного листа, если для всех Atomic'ов он окажется одинаков.
{
    char name[32] = {0};//локальный массив для имени, string неконстантный тип (не позволяет писать напрямую)
    this->psp_material_string_array.reset(new std::string * [this->cnt_of_atomics]);

    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        this->psp_material_string_array[n] = new std::string[this->num_of_materials[n]];

        for (unsigned int i = 0; i < this->num_of_materials[n]; ++i) {
            if (this->psp_material_list_ofst[n][i] != 0x00) {
                memset(name, 0, 32);// затираем буфер с прошлой итерации
                fseek(mdl_file, psp_material_list_ofst[n][i], SEEK_SET);
                fscanf(mdl_file, "%32c", name);
                psp_material_string_array[n][i] = std::string(name);
            } else {
                psp_material_string_array[n][i] = std::string("NULL");
            }
        }
    }

    /// Намеренно оставленный дебаг списка материалов
    ///debug = fopen("debug.bin", "wb+");
    ///for (unsigned int d = 0; d < this->cnt_of_atomics; d++) {
    ///    fwrite(this->psp_material_string_array[d], sizeof(string), this->num_of_materials[d], debug);
    ///}
    ///fclose(debug);

    GetGlobalMaterials(psp_material_string_array.get());//получаем полный и понятный список текстур

    return psp_material_string_array.get();
}

ModelInfo::SubMeshHeader** ModelSimple::GetSubMeshHeaders()
{
    this->sub_mesh_header_list.reset(new SubMeshHeader * [this->cnt_of_atomics]);

    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        this->sub_mesh_header_list[n] = new SubMeshHeader[header[n].num_sub_meshes];
        fseek(mdl_file, psp_geometry_ofst[n] + 104, SEEK_SET);                                        //переходим на первый сабмеш

        fread(&sub_mesh_header_list[n][0], sizeof(SubMeshHeader), header[n].num_sub_meshes, mdl_file);//пробуем прочесть сразу все сабмеши
    }

    return sub_mesh_header_list.get();
}