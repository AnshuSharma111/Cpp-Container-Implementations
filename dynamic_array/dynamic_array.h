#pragma once

#include <algorithm>
#include <stdexcept>
#include <type_traits>

template <typename T>
class DynamicArray {
private:
    size_t _size;
    size_t _capacity;
    T* _array;

    // function to resize array
    void resize(size_t new_capacity) {
        // new T() => allocate memory + manage lifetime
        // ::operator new => only allocate memory
        // new loc T() => gice allocated memory loc, construct object there

        T* new_array = static_cast<T*>(::operator new[](new_capacity * sizeof(T))); // get raw memory

        size_t i = 0;
        try {
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                for (; i < _size; i++) {
                    new (new_array + i) T(std::move(_array[i])); // placement new to construct object
                }
            }
            else {
                for (; i < _size; i++) {
                    new (new_array + i) T(_array[i]);
                }
            }
        }
        catch (...) {
            // rollback in case of failure
            for (; i > 0; i--) {
                new_array[i-1].~T(); // call destructor manually for placement new constructed memory
            }
            ::operator delete[](static_cast<void*>(new_array));
            throw;
        }

        for (size_t j = 0; j < _size; j++) {
            _array[j].~T();
        }

        ::operator delete[](static_cast<void*>(_array));
        _capacity = new_capacity;
        _array = new_array;
    }

    // double array size
    void grow() {
        size_t new_capacity = std::max(size_t(4), _capacity * 2);
        resize(new_capacity);
    }

    // half array size
    void shrink() {
        size_t new_capacity = std::max(size_t(4), _capacity / 2);
        resize(new_capacity);
    }

public:
    // default constructor
    DynamicArray() {
        _capacity = 1;
        _size = 0;
        _array = static_cast<T*>(::operator new(_capacity * sizeof(T), std::align_val_t(alignof(T))));
    }

    // default constructor capacity override
    DynamicArray(size_t capacity) {
        // deep questions, should 0 capacity be allowed?
        if (capacity == 0) {
            throw std::invalid_argument("capacity must > 0!");
        }

        _capacity = capacity;
        _size = 0;
        
        // accquire memory (no construction)
        _array = static_cast<T*>(::operator new(_capacity * sizeof(T), std::align_val_t(alignof(T))));
    }

    // default constructor capacity, default override
    DynamicArray(size_t capacity, const T& x) {
        static_assert(std::is_copy_constructible_v<T>, "type is not copyable!");

        if (capacity == 0) {
            throw std::invalid_argument("capacity must > 0!");
        }

        _capacity = capacity;
        _size = capacity;

        // accquire memory
        _array = static_cast<T*>(::operator new(_capacity * sizeof(T), std::align_val_t(alignof(T))));
        // now construct
        size_t i = 0;
        try {
            for (; i < _capacity; i++) {
                new (_array + i) T(x); // assumes T has a copy constructor
            }
        }
        catch (...) {
            for (; i > 0; i--) {
                _array[i - 1].~T();
            }
            ::operator delete(_array, std::align_val_t(alignof(T)));
            throw;
        }
    }

    // Destructor
    ~DynamicArray() {
        delete[] _array;
    }

    // copy constructor
    DynamicArray(const DynamicArray& other) {
        _size = other._size;
        _capacity = other._capacity;

        _array = new T[_capacity];
        std::copy(other._array, other._array + _size, _array);
    }

    // assignment operator
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            T* newArray = new T[other._capacity];
            std::copy(other._array, other._array + _size, newArray);

            delete[] _array;

            _size = other._size;
            _capacity = other._capacity;
            _array = newArray;
        }

        return *this;
    }

    // move constructor
    DynamicArray(DynamicArray&& other) noexcept {
        _size = other._size;
        _capacity = other._capacity;
        _array = other._array;

        other._array = nullptr;
        other._capacity = 0;
        other._size = 0;
    }

    // move assignment
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] _array;

            _size = other._size;
            _capacity = other._capacity;
            _array = other._array;

            other._size = 0;
            other._capacity = 0;
            other._array = nullptr;
        }

        return *this;
    }

    // push by const lvalue
    void push(const T& x) {
        if (_size == _capacity) grow();
        _array[_size++] = x;
    }

    // push by rvalue binding
    void push(T&& x) {
        if (_size == _capacity) grow();
        _array[_size++] = std::move(x);
    }
};