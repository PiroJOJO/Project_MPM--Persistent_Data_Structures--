#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <stdexcept>
#include <functional>

// PersistentVector
template<typename T>
class SimplePersistentVector {
private:
    std::shared_ptr<std::vector<T>> data;

public:
    SimplePersistentVector() : data(std::make_shared<std::vector<T>>()) {}

    size_t size() const { return data->size(); }
    bool empty() const { return data->empty(); }

    const T& operator[](size_t index) const { return (*data)[index]; }

    SimplePersistentVector<T> append(const T& value) const {
        SimplePersistentVector<T> new_vec;
        new_vec.data = std::make_shared<std::vector<T>>(*data);
        new_vec.data->push_back(value);
        return new_vec;
    }

    SimplePersistentVector<T> set(size_t index, const T& value) const {
        if (index >= size()) throw std::out_of_range("Index out of range");

        SimplePersistentVector<T> new_vec;
        new_vec.data = std::make_shared<std::vector<T>>(*data);
        (*new_vec.data)[index] = value;
        return new_vec;
    }
};

//  PersistentList
template<typename T>
class SimplePersistentList {
private:
    struct Node {
        T value;
        std::shared_ptr<Node> next;
        Node(const T& v, std::shared_ptr<Node> n = nullptr) : value(v), next(n) {}
    };

    std::shared_ptr<Node> head;
    size_t list_size;

public:
    SimplePersistentList() : head(nullptr), list_size(0) {}

    size_t size() const { return list_size; }
    bool empty() const { return list_size == 0; }

    const T& front() const {
        if (empty()) throw std::runtime_error("List is empty");
        return head->value;
    }

    SimplePersistentList<T> prepend(const T& value) const {
        SimplePersistentList<T> new_list;
        new_list.head = std::make_shared<Node>(value, head);
        new_list.list_size = list_size + 1;
        return new_list;
    }
};

int main() {
    std::cout << "=== Simple Persistent Vector Demo ===" << std::endl;

    SimplePersistentVector<int> vec1;
    auto vec2 = vec1.append(10).append(20).append(30);
    auto vec3 = vec2.set(1, 99);

    std::cout << "vec2[1] = " << vec2[1] << std::endl; // 20
    std::cout << "vec3[1] = " << vec3[1] << std::endl; // 99
    std::cout << "vec2 size = " << vec2.size() << std::endl;
    std::cout << "vec3 size = " << vec3.size() << std::endl;

    std::cout << "\n=== Simple Persistent List Demo ===" << std::endl;

    SimplePersistentList<std::string> list1;
    auto list2 = list1.prepend("world").prepend("hello");

    std::cout << "List size: " << list2.size() << std::endl;
    std::cout << "First element: " << list2.front() << std::endl;

    return 0;
}