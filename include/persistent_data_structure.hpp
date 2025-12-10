#ifndef PERSISTENT_DATA_STRUCTURE_HPP
#define PERSISTENT_DATA_STRUCTURE_HPP

#include <memory>
#include <cstddef>

// -----------------------------------------
// ---------- Единый API структур ----------
// -----------------------------------------
// 
// Определяет единый API для всех реализованных 
// структур с общими утилитами

template<typename T>
class IPersistentStructure {
public:
    virtual ~IPersistentStructure() = default;

    // -----------------------------------------
    // ----------- Размер структуры ------------
    // -----------------------------------------
    virtual size_t size() const = 0;

    // -----------------------------------------
    // ----------- Очистка структуры -----------
    // -----------------------------------------
    virtual std::shared_ptr<IPersistentStructure<T>> clear() const = 0;

    // -----------------------------------------
    // ----- Проверка на пустоту структуры -----
    // -----------------------------------------
    virtual bool empty() const = 0;

    // -----------------------------------------
    // --- Создание глубокой копии структуры ---
    // -----------------------------------------
    virtual std::shared_ptr<IPersistentStructure<T>> clone() const = 0;
};

#endif 