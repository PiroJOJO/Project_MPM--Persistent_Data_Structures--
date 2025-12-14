cmake_minimum_required(VERSION 3.10)
project(PersistentDataStructures)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Устанавливаем путь к заголовкам
include_directories(include)

# Библиотека persistent_data_structures (header-only, так как шаблонные)
add_library(persistent_data_structures INTERFACE)
target_include_directories(persistent_data_structures INTERFACE include)

# Если есть .cpp файлы для библиотеки, добавьте их здесь
# Например:
if(EXISTS "src/persistent_value.cpp")
    # Создаем отдельную библиотеку для .cpp файлов
    add_library(persistent_value src/persistent_value.cpp)
    target_include_directories(persistent_value PUBLIC include)
endif()

# Загружаем Google Test через URL (ZIP архив)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Устанавливаем опции для Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Загружаем googletest
FetchContent_MakeAvailable(googletest)

# Тестовое приложение - правильный путь к main.cpp
add_executable(persistent_tests 
    src/main.cpp
    src/persistent_value.cpp
)

# Добавляем persistent_value.cpp к тестам, если он существует
if(EXISTS "src/persistent_value.cpp")
    target_sources(persistent_tests PRIVATE src/persistent_value.cpp)
endif()

# Линкуем с Google Test и нашей библиотекой
target_link_libraries(persistent_tests
    GTest::gtest_main
    persistent_data_structures
)

# Если создали библиотеку persistent_value, линкуем с ней
if(TARGET persistent_value)
    target_link_libraries(persistent_tests persistent_value)
endif()

# Включаем директорию с заголовками для тестов
target_include_directories(persistent_tests PRIVATE include)

# Для удобства - цель для запуска тестов
add_custom_target(run_tests
    COMMAND persistent_tests
    DEPENDS persistent_tests
)

# Дополнительные цели для демо-приложений
if(EXISTS "out/src/single_file_demo.cpp")
    add_executable(single_file_demo out/src/single_file_demo.cpp)
    target_link_libraries(single_file_demo persistent_data_structures)
    if(TARGET persistent_value)
        target_link_libraries(single_file_demo persistent_value)
    endif()
    target_include_directories(single_file_demo PRIVATE include)
endif()