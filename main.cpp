///#define _CRTDBG_MAP_ALLOC
///#define _CRTDBG_MAP_ALLOC_NEW
#include <cstdlib>
///#include <crtdbg.h>
///#include <assert.h>
#include <iomanip>//исп setprecision(для формат вывод данных)
#include "model_info.h"

using std::fixed;
using std::setprecision;
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::uppercase;
using std::setw;

int main(int argc, char* argv[])
{
    //_crtBreakAlloc = 97;
    cout << "MDL_Helper_0.2.3 by DenielX\n";

    ModelInfo::TypeMDL detected_type = ModelInfo::TypeMDL::unknown;

    // Фабрика теперь возвращает std::unique_ptr, память будет освобождена автоматически
    std::unique_ptr<ModelInfo> global_clump = ModelInfo::CreateModelMDL(argc, argv, detected_type);

    if (!global_clump) {
        cout << "Temporary unsupported .mdl type...\n";
    }

    switch (detected_type) {
    case ModelInfo::TypeMDL::SimpleModel:
        cout << "MDL Type:                     SimpleModel\n";
        break;
    case ModelInfo::TypeMDL::Vehicle:
        cout << "MDL Type:                     Vehicle\n";
        break;
    case ModelInfo::TypeMDL::Peds:
        cout << "MDL Type:                     Ped\n";
        break;
    case ModelInfo::TypeMDL::csPedsObj:
        cout << "MDL Type:                     Cutscene Obj or csPed\n";
        break;
    case ModelInfo::TypeMDL::unknown:
        cout << "MDL Type:                     Unknown type\n";
        break;
    }

    if (!global_clump) {
        system("pause");
        return 1;
    }

    if (global_clump->type_mdl == ModelInfo::TypeMDL::SimpleModel) {
        cout << "Total Atomics in MDL:         " << dec << global_clump->cnt_of_atomics << endl;
        cout << "Global Material List:\n";

        for (std::string temp_str : global_clump->global_material_list) {
            cout << temp_str << endl;
        }

        for (unsigned short atms = 0; atms < global_clump->cnt_of_atomics; ++atms) {
            cout << "\n " << setw(35) << "------ATOMIC #" << atms << ":------" << setw(35) << " " << endl;
            cout << "Atomic offset #" << atms << ":             0x" << hex << uppercase << global_clump->psp_atomic_ofst[atms] << endl;
            cout << "Geometry offset #" << atms << ":           0x" << global_clump->psp_geometry_ofst[atms] << endl;
            cout << "GeometryHeader offset #" << atms << ":     0x" << global_clump->geometry_header_ofst[atms] << endl;

            // Вывод флагов через тернарный оператор для компактности
            cout << "MDL flags in atom #" << atms << ": "
                << (global_clump->bits_flags[atms].uvfmt != 0 ? "UV map; " : "")
                << (global_clump->bits_flags[atms].colfmt != 0 ? "Prelight; " : "")
                << (global_clump->bits_flags[atms].normfmt != 0 ? "Normals; " : "")
                << (global_clump->bits_flags[atms].posfmt != 0 ? "Vertices; " : "")
                << (global_clump->bits_flags[atms].wghtfmt != 0 ? "Weight(for skin); " : "");

            if (global_clump->bits_flags[atms].nwght != 0) {
                cout << "Num of weight(on one vert)=" << global_clump->bits_flags[atms].nwght << "; ";
            }
            if (global_clump->bits_flags[atms].nvert != 0) {
                cout << "Num of vert(on one morf)=" << global_clump->bits_flags[atms].nvert << "; ";
            }

            cout << (global_clump->bits_flags[atms].idxfmt != 0 ? "Indexes; " : "");

            cout << "\n\n" << setw(41) << "---MESH INFO:---" << setw(30) << endl;
            cout << "Mesh offset:                  " << "0x" << hex << unsigned int(global_clump->header[atms].psp_geometry_mesh_ofst) << endl;
            cout << "Number of mat/submesh:        " << global_clump->num_of_materials[atms] << "/" << global_clump->header[atms].num_sub_meshes << endl;
            cout << "Total vertices:               " << dec << global_clump->header[atms].sum_of_vertices << endl;
            cout << "Global ScaleMatrix:           " << fixed << setprecision(2) <<
                "x: " << global_clump->header[atms].sum_scale[0] <<
                "f y: " << global_clump->header[atms].sum_scale[1] <<
                "f z: " << global_clump->header[atms].sum_scale[2] << "f\n";
            cout << "Global TranslateMatrix:       " << fixed << setprecision(2) <<
                "x: " << global_clump->header[atms].sum_translate[0] <<
                "f y: " << global_clump->header[atms].sum_translate[1] <<
                "f z: " << global_clump->header[atms].sum_translate[2] << "f\n";
            cout << "Bounding Sphere (center,rad): " << fixed << setprecision(2) <<
                "x: " << global_clump->header[atms].bound_sphere[0] <<
                "f y: " << global_clump->header[atms].bound_sphere[1] <<
                "f z: " << global_clump->header[atms].bound_sphere[2] <<
                "f radius: " << global_clump->header[atms].bound_sphere[3] << "f\n";
            cout << "\n\n";

            cout << " " << setw(39) << "-SUBMESH INFO:-" << " " << setw(31) << "\n\n";
            for (unsigned int i = 0; i < global_clump->header[atms].num_sub_meshes; ++i) {
                cout << "SubMesh #" << i << ":\n";
                cout << "TexName and MatID:            " << global_clump->psp_material_string_array[atms][global_clump->sub_mesh_header_list[atms][i].mat_id] << " :: " <<
                    global_clump->sub_mesh_header_list[atms][i].mat_id << endl;
                cout << "Vertices/polygons on submesh: " << dec << global_clump->sub_mesh_header_list[atms][i].triangles_on_sub + 2 << "/" <<
                    global_clump->sub_mesh_header_list[atms][i].triangles_on_sub << endl;
                cout << "uScale and vScale:            " << fixed << setprecision(2) << global_clump->sub_mesh_header_list[atms][i].u_scale << "f " <<
                    global_clump->sub_mesh_header_list[atms][i].v_scale << "f\n";
                cout << "SubMesh Offset(from MeshPtr): " << "0x" << hex << unsigned int(global_clump->sub_mesh_header_list[atms][i].to_sub_mesh_ofst) << endl;
                cout << "Material offset:              " << "0x" << hex << global_clump->psp_material_list_ofst[atms][global_clump->sub_mesh_header_list[atms][i].mat_id] << endl << endl;
            }
        }
    }

    system("pause");

    return 0;
}