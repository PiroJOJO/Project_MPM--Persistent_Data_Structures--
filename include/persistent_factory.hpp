#ifndef PERSISTENT_FACTORY_HPP
#define PERSISTENT_FACTORY_HPP

#include "persistent_vector.hpp"
#include "persistent_list.hpp"
#include "persistent_map.hpp"
#include <vector>
#include <iostream>

class PersistentFactory {
public:
    // -----------------------------------------
    // --- PersistentList в PersistentVector ---
    // -----------------------------------------
    template<typename T>
    static PersistentVector<T> listToVector(const PersistentList<T>& list) {
        PersistentVector<T> result;
        std::cout << "DEBUG: listToVector called, list size = " << list.size() << std::endl;

        // Через итераторы
        try {
            auto it = list.begin();
            auto end = list.end();
            while (it != end) {
                result = result.append(*it);
                ++it;
            }
            return result;
        }
        catch (...) {
        }

        // Через Container
        try {
            auto temp = list.template toContainer<std::vector<T>>();
            return PersistentVector<T>(temp);
        }
        catch (...) {
        }
        return result;
    }

    // -----------------------------------------
    // --- PersistentVector в PersistentList ---
    // -----------------------------------------
    template<typename T>
    static PersistentList<T> vectorToList(const PersistentVector<T>& vector) {
        PersistentList<T> result;

        std::cout << "DEBUG: vectorToList called, vector size = " << vector.size() << std::endl;

        for (size_t i = vector.size(); i > 0; --i) {
            try {
                // Через operator[]
                result = result.prepend(vector[i - 1]);
            }
            catch (...) {
                try {
                    // Через get()
                    result = result.prepend(vector.get(i - 1));
                }
                catch (...) {
                    std::cerr << "ERROR: Cannot access vector element at index " << (i - 1) << std::endl;
                    throw;
                }
            }
        }
        return result;
    }

    // -----------------------------------------
    // ---- PersistentMap в PersistentVector ---
    // -----------------------------------------
    template<typename K, typename V>
    static PersistentVector<std::pair<K, V>> mapToVector(const PersistentMap<K, V>& map) {
        PersistentVector<std::pair<K, V>> result;
        std::cout << "DEBUG: mapToVector called" << std::endl;

        // Через итераторы
        try {
            auto it = map.begin();
            auto end = map.end();
            while (it != end) {
                result = result.append(*it);
                ++it;
            }
        }
        catch (...) {
            std::cerr << "WARNING: Cannot iterate over PersistentMap" << std::endl;
        }
        return result;
    }

    // -----------------------------------------
    // ---- PersistentMap в PersistentList ----
    // -----------------------------------------
    template<typename K, typename V>
    static PersistentList<std::pair<K, V>> mapToList(const PersistentMap<K, V>& map) {
        PersistentList<std::pair<K, V>> result;

        std::cout << "DEBUG: mapToList called" << std::endl;

        // Сначала преобразуем в вектор, затем в список
        auto vec = mapToVector(map);
        return vectorToList(vec);
    }

    // -----------------------------------------
    // ---- PersistentList в PersistentMap -----
    // -----------------------------------------
    template<typename K, typename V>
    static PersistentMap<K, V> vectorToMap(const std::vector<std::pair<K, V>>& vec) {
        PersistentMap<K, V> result;
        std::cout << "DEBUG: vectorToMap called, vector size = " << vec.size() << std::endl;

        for (const auto& pair : vec) {
            result = result.set(pair.first, pair.second);
        }
        return result;
    }

    // -----------------------------------------
    // --- PersistentVector в PersistentMap ----
    // -----------------------------------------
    template<typename K, typename V>
    static PersistentMap<K, V> persistentVectorToMap(const PersistentVector<std::pair<K, V>>& vec) {
        PersistentMap<K, V> result;
        std::cout << "DEBUG: persistentVectorToMap called, vector size = " << vec.size() << std::endl;
        // PersistentVector -> std::vector -> vectorToMap
        std::vector<std::pair<K, V>> temp;

        // Через итераторы
        try {
            auto it = vec.begin();
            auto end = vec.end();
            while (it != end) {
                temp.push_back(*it);
                ++it;
            }
        }
        catch (...) {
            for (size_t i = 0; i < vec.size(); ++i) {
                try {
                    temp.push_back(vec[i]);
                }
                catch (...) {
                    try {
                        temp.push_back(vec.get(i));
                    }
                    catch (...) {
                        std::cerr << "ERROR: Cannot access vector element" << std::endl;
                        throw;
                    }
                }
            }
        }
        return vectorToMap(temp);
    }
};

#endif 