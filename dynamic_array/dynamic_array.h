#pragma once

#include <algorithm>
#include <stdexcept>

template <typename T>
class DynamicArray {
private:
    size_t size;
    size_t capacity;
    T* array;

    // function to double array size
    void grow() {
        size_t new_capacity = std::max(size_t(4), capacity * 2);
        T* new_array = nullptr;

        try {
            new_array = new T[new_capacity];
            std::move(array, array + size, new_array);
        }
        catch (...) {
            delete[] new_array;
            throw;
        }

        delete[] array;
        array = new_array;
        capacity = new_capacity;
    }

    // function to half array size
    void shrink() {
        size_t new_capacity = std::max(size_t(4), capacity / 2);
        T* new_array = nullptr;

        try {
            new_array = new T[new_capacity];
            std::move(array, array + size, new_array);
        }
        catch (...) {
            delete[] new_array;
            throw;
        }

        delete[] array;
        array = new_array;
        capacity = new_capacity;
    }

public:
    // constructor
    DynamicArray() : size(0), capacity(4), array(new T[4]) {}

    // Destructor
    ~DynamicArray() {
        delete[] array;
    }

    // copy constructor
    DynamicArray(const DynamicArray& other) {
        size = other.size;
        capacity = other.capacity;

        array = new T[capacity];
        std::copy(other.array, other.array + size, array);
    }

    // assignment operator
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            T* newArray = new T[other.capacity];
            std::copy(other.array, other.array + size, newArray);

            delete[] array;

            size = other.size;
            capacity = other.capacity;
            array = newArray;
        }

        return *this;
    }

    // move constructor
    DynamicArray(DynamicArray&& other) noexcept {
        size = other.size;
        capacity = other.capacity;
        array = other.array;

        other.array = nullptr;
        other.capacity = 0;
        other.size = 0;
    }

    // move assignment
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] array;

            size = other.size;
            capacity = other.capacity;
            array = other.array;

            other.size = 0;
            other.capacity = 0;
            other.array = nullptr;
        }

        return *this;
    }

    // push by const lvalue
    void push(const T& x) {
        if (size == capacity) grow();
        array[size++] = x;
    }

    // push by rvalue binding
    void push(T&& x) {
        if (size == capacity) grow();
        array[size++] = std::move(x);
    }

    // pop last element
    T pop() {
        if (size == 0) throw std::runtime_error("Array is empty!");

        T ret_val = std::move(array[size - 1]);
        size--;

        if (capacity > 4 && size <= capacity / 4) shrink();
        return ret_val;
    }

    // pop by index
    T pop(size_t idx) {
        if (size == 0) throw std::runtime_error("Array is empty!");
        if (idx >= size) throw std::out_of_range("Index out of range!");

        T ret_val = std::move(array[idx]);
        size--;

        for (size_t i = idx; i < size; i++) {
            array[i] = std::move(array[i + 1]);
        }

        if (capacity > 4 && size <= capacity / 4) shrink();
        return ret_val;
    }

    // insert at index using const lvalue reference
    void insert(const T& x, size_t idx) {
        if (idx > size) throw std::out_of_range("Index out of range!");
        if (size == capacity) grow();

        for (size_t i = size; i > idx; i--) {
            array[i] = std::move(array[i - 1]);
        }

        array[idx] = x;
        size++;
    }

    // insert at index using rvalue binding
    void insert(T&& x, size_t idx) {
        if (idx > size) throw std::out_of_range("Index out of range!");
        if (size == capacity) grow();

        for (size_t i = size; i > idx; i--) {
            array[i] = std::move(array[i - 1]);
        }

        array[idx] = std::move(x);
        size++;
    }

    // get array length
    size_t length() const {
        return size;
    }

    // is array empty
    bool empty() const {
        return size == 0;
    }

    // access element by index
    T& operator[](size_t idx) {
        if (idx >= size) throw new std::out_of_range("Index out of range!");
        return array[idx];
    }

    // access element by index (const)
    const T& operator[](size_t idx) const {
        if (idx >= size) throw new std::out_of_range("Index out of range!");
        return array[idx];
    }

    // begin pointer
    T* begin() {
        return array;
    }

    // end pointer
    T* end() {
        return array + size;
    }

    // const begin pointer
    const T* begin() const {
        return array;
    }

    // const end pointer
    const T* end() const {
        return array + size;
    }
};