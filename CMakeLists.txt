```cmake
cmake_minimum_required(VERSION 3.10)

# Имя проекта и версия
project(MDL_Helper LANGUAGES CXX)

# C++11 минимально для умных указателей и enum class
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Указываем директорию с заголовочными файлами
include_directories(include)

# Собираем все исходники из папки src/
file(GLOB SOURCES "src/*.cpp")

# Добавляем точку входа
list(APPEND SOURCES "main.cpp")

# Создаем исполняемый файл
add_executable(MDL_Helper ${SOURCES})

# Если это MSVC (Visual Studio), добавляем специфичные флаги для подавления старых C-ворнингов (опционально)
if(MSVC)
    target_compile_definitions(MDL_Helper PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()