#ifndef MODEL_INFO_H
#define MODEL_INFO_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <memory>

class ModelInfo
{
public:
    // Виртуальный деструктор обязателен для базового класса
    virtual ~ModelInfo() {
        if (mdl_file) {
            fclose(mdl_file);
        }
        std::cout << "Base Destructor finished.\n";
    }

    enum class TypeMDL {
        SimpleModel = 0, // У этого типа Clump'а нету, проверено во всех вики и доступных исходниках + нейронка. Тупо массив атомиков.
        Vehicle = 1,
        Peds = 2,
        csPedsObj = 3,
        unknown = 4
    };

    struct SubMeshHeader // 48 byte
    {
        unsigned int* to_sub_mesh_ofst; //указатель на первый вертекс сабмеша (относительно psp_geometry_mesh_ofst)
        unsigned short triangles_on_sub;//кол-во треугольников в сабмеше.triangles_on_sub+2=кол-во вертексов в сабмеше
        unsigned short mat_id;          //id материала сабмеша
        float unk1;                     //пока неизвестно
        float u_scale;                  //множитель U координат развертки сабмеша
        float v_scale;                  //аналогично
        float unk2[5];                  //возможно, BoundBox сабмеша
        unsigned char unk3[8];          //карта костей сабмеша?
    };
    std::unique_ptr<SubMeshHeader* []> sub_mesh_header_list;

    struct GeometryHeader {
        unsigned int summary_size;    // суммарный размер данных, начиная прямо от этой переменной
                                      // и до конца всего фактического меша (без служебки после него)*/
        unsigned int psp_flags;       // Тут вся битовая маска конфига меша:
                                      // uvfmt,colfmt,normfmt,posfmt,wghtfmt,idxfmt,nwght,и другие(см. struct Flags).
        
        unsigned int num_sub_meshes;  //кол-во сабмешей в одном Atomic модели
        unsigned int unk1;
        float bound_sphere[4];        //ограничивающая сфера модели:x,y,z,радиус
        float sum_scale[3];           //общее масштабирование модели (x,y,z)
        int sum_of_vertices;          //суммарное кол-во точек в модели
        float sum_translate[3];       //общий сдвиг модели(x,y,z)
        int unk2;
        unsigned int* psp_geometry_mesh_ofst;//совпадает по значению в librw, но у меня указатель позднее (сразу на данные пересчитан)
        float unk3;
    };
    std::unique_ptr<GeometryHeader[]> header;

    struct Flags {
        int uvfmt;        //2bits,флаг формата UV: 0-нет,1-по одному байту fixed point на U и V,2-по 2b fixed point, 3-по floatу каждому
        int colfmt;       //3bits,флаг формата vcol(prel): 0-нет,4=BGR-5650,5=ABGR-5551,6=ABGR-4444,7=ABGR-8888 и др.
        int normfmt;      //2bits,флаг формата normals: 0-нет,1-по одному байту fixed point на X&Y&Z,2-по 2b fixed point, 3-по floatу каждому
        int posfmt;       //2bits,флаг формата XYZ вертекса: аналогично флагу нормалей
        int wghtfmt;      //2bits,флаг формата weight(весов): аналогично флагу нормалей
        int idxfmt;       //2bits,флаг формата индексов: 0-нет,1=ubyte8,2=ubyte16,3=не используется,для доп.команды GPU
        int nwght;        //3bits,флаг кол-ва весов(костей) на одну вершину.От 1 до 8 включительно.
        int nvert;        //3bits,флаг кол-ва целей для 1 морфинга.1-8(вкл) вершин на морфинг.
        int twopassfmt;   //1bits,флаг двупроходного конвеера.0-модель трансформирована в матрицу мира(2pass-OFF).
                          //1-модель в своём первозданном виде, требуется умножение на матрицу мира(2pass-ON)
    };//uvfmt - нулевой по порядку бит в маске, twopassfmt - 23 бит.Биты 21-22 не используются.24-31 GPU команда 'VTYPE'
    std::unique_ptr<Flags[]> bits_flags;

    TypeMDL type_mdl;                     //тип mdl, для себя TODO: проверить, возможно deprecated
    unsigned short cnt_of_atomics;        //кол-во атомиков на Clump

    std::unique_ptr<unsigned int[]> psp_atomic_ofst;            //rslElement в librw
    std::unique_ptr<unsigned int[]> psp_geometry_ofst;          //rslGeometry в librw
    std::unique_ptr<unsigned int* []> psp_material_list_ofst;   //часть rslMaterial(только названия текстур).
    std::unique_ptr<unsigned int[]> geometry_header_ofst;       //aka GeometryHeader:Struct с gtamodding.ru или sPspGeometry в librw
    std::unique_ptr<unsigned int[]> num_of_materials;           //для кол-ва текстур на один Atomic
    unsigned short sum_of_vertices = 0;                         //для кол-ва суммарных точек в модели
    std::unique_ptr<std::string* []> psp_material_string_array; //В конечном итоге двумерный массив string. 1-ая размерность - номер Atomic,
                                        // 2-ая - номер строки материала.В librw, безымянный
                                        //union c char *texname и RslTexture *texture (принадлежит RslMaterial).
                                        //Что интересно - материалы должны парсится по порядку rslNode, а не подряд.
    std::vector<std::string> global_material_list;//общий список материалов в модели со всех Atomic'ов без повторов.

    // Чисто виртуальные методы, которые должны реализовать наследники
    virtual unsigned int* GetAtomicOfst() = 0;       //получаем указатель на атомик(rslElement = 0x01)
    virtual unsigned int* GetGeometryOfst() = 0;     //на его основании, получаем указатель на геометрию(rslGeometry = 0x08)
    virtual GeometryHeader* GetGeometryHeader() = 0; //по сути получаем адрес Clump->Struct и структуру
    virtual unsigned int** GetMaterialOfst() = 0;    //вычисляем адрес списка материалов по цепочке указателей, стартующей в rslGeometry
    virtual std::string** GetMaterialList() = 0;     //читаем строки материалов по данному указателю
    virtual SubMeshHeader** GetSubMeshHeaders() = 0; //находим хедеры сабмешей (aka sPspGeometryMesh в librw)

    // Метод с заглушкой. Наследники, у которых есть Clump (Vehicle, Peds), переопределят его.
    virtual unsigned int* GetClumpOfst() { return nullptr; }

    // Фабричный метод для создания нужного типа модели (возвращает умный указатель)
    static std::unique_ptr<ModelInfo> CreateModelMDL(int argc, char* argv[], TypeMDL& out_type);

protected:
    FILE* mdl_file;
    std::unique_ptr<unsigned int> psp_clump_ofst;//сам Clump (rslElementGroup == 0x02)

    // Защищенный конструктор, чтобы объект создавался только через фабрику
    ModelInfo(FILE* file, unsigned short cnt_atomics);

    // Общие вспомогательные методы
    void GetFlags(unsigned int* psp_flags);
    std::vector<std::string> GetGlobalMaterials(std::string** material_lst);
    // Статический метод анализа файла до создания объекта
    static TypeMDL TypeDetectMDL(FILE* file, unsigned short& out_cnt);
};

#endif // MODEL_INFO_H
/*
 enum PSP_VTYPE{
 		POINTS = 0,
		LINES = 1,
		LINE_STRIP = 2,    //вычисляется на рантайме, в реальных файлах моделей отсутствует.
        TRIANGLES = 3,     //единичные случаи в дебагере(где-то ~2500 примитивов было в сцене).Не видел в mdl в натуральном виде.
		TRIANGLE_STRIP = 4,//в моделях почти всегда он
		TRIANGLE_FAN = 5,  //иногда он (<1% относительно всех)
		RECTANGLES = 6,    //используется для рендеринга освещения и подобного, в отдельном виде (.mdl/.wrld) не встречается (мб вшита в mdl как 2dfx?)
    Исходя из поддержки в исходном коде librw (только TRIANGLE_STRIP),
    напрашивается вывод, что поддержка других типов - пустая трата времени.
    Однако на финальной стадии разработки есть смысл пакетно проверить этот флаг во всех .mdl и WRLD.
    Возможно найдутся TRIANGLES и TRIANGLE_FAN
 };
*/
