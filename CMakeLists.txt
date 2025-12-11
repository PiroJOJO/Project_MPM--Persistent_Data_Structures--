cmake_minimum_required(VERSION 3.10)
project(PersistentDataStructures)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем путь к заголовочным файлам
include_directories(include)

# Основная библиотека
add_library(persistent_data_structures STATIC
    src/persistent_value.cpp
 "single_file_demo.cpp")

# Тестовое приложение
add_executable(persistent_demo
    "src/main.cpp"
 "single_file_demo.cpp")

target_link_libraries(persistent_demo
    persistent_data_structures
)