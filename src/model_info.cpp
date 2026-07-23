#include "model_info.h"
#include "model_simple.h"
#include "model_vehicle.h"

// Конструктор базового класса только сохраняет данные
ModelInfo::ModelInfo(FILE* file, unsigned short cnt_atomics)
    : mdl_file(file), cnt_of_atomics(cnt_atomics), type_mdl(TypeMDL::unknown)
{}

std::unique_ptr<ModelInfo> ModelInfo::CreateModelMDL(int argc, char* argv[], TypeMDL& out_type)
{
    FILE* file = nullptr;
    if (argc > 1) {                      
        file = fopen(argv[1], "rb+");    //открываем модель drag-n-drop
        std::cout << argv[1] << "\n";
    } else {
        file = fopen("model.mdl", "rb+");//открываем модель по умолчанию
        std::cout << "model.mdl is opened...\n";
    }

    if (!file) {
        out_type = TypeMDL::unknown;
        std::cout << "Error opening file!\n";
        return nullptr;
    }

    unsigned short cnt_atomics = 0;
    // определяем тип модели по заголовку и секциям:
    out_type = TypeDetectMDL(file, cnt_atomics);

    switch (out_type) {
    case TypeMDL::SimpleModel:
        return std::unique_ptr<ModelInfo>(new ModelSimple(file, cnt_atomics));
    case TypeMDL::Vehicle:
        return std::unique_ptr<ModelInfo>(new ModelVehicle(file, cnt_atomics));
    default:
        fclose(file);
        return nullptr;
    }
}

ModelInfo::TypeMDL ModelInfo::TypeDetectMDL(FILE* file, unsigned short& out_cnt)
{
    //временные каретки
    std::unique_ptr<unsigned int> addr1(new unsigned int);
    std::unique_ptr<unsigned int> addr2(new unsigned int); 
    unsigned char flg;                               //по сути это uint8, для проверки флага

    fseek(file, 0x1E, SEEK_SET);                     //переходим на заголовок DUMP-контейнера MDL
    fread(&out_cnt, sizeof(unsigned short), 1, file);//запоминаем сколько у нас Atomic'ов

    fread(addr1.get(), sizeof(unsigned int*), 1, file);
    switch (*addr1) {
    case 2: //Объекты и педы катсцен
        return TypeMDL::csPedsObj;
    default:
        fseek(file, *addr1, SEEK_SET);//переходим на предполагаемый Clump или Atomic
        fread(&flg, sizeof(unsigned char), 1, file);
        switch (flg) {
        case 1:
            return TypeMDL::SimpleModel;
        case 2:
            return TypeMDL::Vehicle;
        }
    }

    fseek(file, 0x24, SEEK_SET);//переходим на 2 параметр заголовка MDL
    fread(addr1.get(), sizeof(unsigned int*), 1, file);
    fseek(file, *addr1, SEEK_SET);//переходим на предполагаемый Clump или его флаг
    fread(addr2.get(), sizeof(unsigned int*), 1, file);
    fseek(file, (*addr1) - 8, SEEK_SET);//переходим на предполагаемый флаг Clump'а
    fread(addr1.get(), sizeof(unsigned int*), 1, file);

    if (*addr1 == 0x02 || *addr2 == 0x02) { //если указатель идет на флаг Clump'а или в секцию Clump
        return TypeMDL::Peds;
    } else {
        return TypeMDL::unknown;
    }
}

std::vector<std::string> ModelInfo::GetGlobalMaterials(std::string** material_lst)
{
    for (int n = 0; n < this->cnt_of_atomics; ++n) {
        for (unsigned int i = 0; i < this->num_of_materials[n]; ++i) {
            bool found = false;
            for (std::string current : this->global_material_list) {
                if (current == material_lst[n][i]) {
                    found = true;
                    break;
                }
            }
            // процедура contains
            // Добавляем строку, если её еще нет в векторе
            if (!found) {
                global_material_list.push_back(material_lst[n][i]);
            }
        }
    }
    return global_material_list;
}

void ModelInfo::GetFlags(unsigned int* psp_flags)
{
    this->bits_flags.reset(new Flags[this->cnt_of_atomics]);
    for (unsigned int n = 0; n < this->cnt_of_atomics; ++n) {
        bits_flags[n].uvfmt = psp_flags[n] & 3;
        bits_flags[n].colfmt = psp_flags[n] >> 2 & 7;
        bits_flags[n].normfmt = psp_flags[n] >> 5 & 3;
        bits_flags[n].posfmt = psp_flags[n] >> 7 & 3;
        bits_flags[n].wghtfmt = psp_flags[n] >> 9 & 3;
        bits_flags[n].idxfmt = psp_flags[n] >> 11 & 3;
        bits_flags[n].nwght = (psp_flags[n] >> 14 & 7) + 1;
        bits_flags[n].nvert = (psp_flags[n] >> 18 & 7) + 1;
        bits_flags[n].twopassfmt = psp_flags[n] >> 23 & 1;
    }
}