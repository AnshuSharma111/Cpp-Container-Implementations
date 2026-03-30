#pragma once

#include <algorithm>
#include <stdexcept>
#include <type_traits>

template <typename T>
class DynamicArray {
private:
    size_t _size = 0;
    size_t _capacity = 0;
    T* _array = nullptr;

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

    // destruct and deallocate memory
    static void deallocate(T* arr, size_t i) {
        if (!arr) return;

        for (; i > 0; i--) {
            arr[i - 1].~T();
        }

        ::operator delete(arr, std::align_val_t(alignof(T)));
    }

public:
    // default constructor
    DynamicArray() noexcept = default; // won't throw ever

    // default constructor
    explicit DynamicArray(size_t capacity) {
        if (capacity == 0) {
            _array = nullptr;
            _size = 0;
            _capacity = 0;
            return;
        }

        _capacity = capacity;
        _size = 0;
        
        // accquire memory (no construction)
        _array = static_cast<T*>(::operator new(_capacity * sizeof(T), std::align_val_t(alignof(T))));
    }

    // default constructor
    DynamicArray(size_t capacity, const T& x) {
        static_assert(std::is_copy_constructible_v<T>, "type is not copy constructible!");

        if (capacity == 0) {
            _array = nullptr;
            _size = 0;
            _capacity = 0;
            return;
        }

        // accquire memory
        _array = static_cast<T*>(::operator new(capacity * sizeof(T), std::align_val_t(alignof(T))));
        // now construct
        size_t i = 0;
        try {
            for (; i < capacity; i++) {
                new (_array + i) T(x); // assumes T has a copy constructor
            }

            // safely constructed, update parameters
            _capacity = capacity;
            _size = capacity;
        }
        catch (...) {
            deallocate(_array, i);
            throw;
        }
    }

    // destructor
    ~DynamicArray() {
        deallocate(_array, _size);
    }

    // copy constructor
    DynamicArray(const DynamicArray& other) {
        static_assert(std::is_copy_constructible_v<T>, "type is not copy constructible!");

        // accquire memory
        _array = static_cast<T*>(::operator new(other._capacity * sizeof(T), std::align_val_t(alignof(T))));

        // try constructing objects
        size_t i = 0;
        try {
            for (; i < other._size; i++) {
                new (_array + i) T(other._array[i]); // T must be copy constructable
            }

            // safely copied
            _size = other._size;
            _capacity = other._capacity;
        }
        catch (...) {
            deallocate(_array, i);
            throw;
        }
    }

    // assignment operator
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            T* new_array = static_cast<T*>(::operator new(other._capacity * sizeof(T), std::align_val_t(alignof(T))));
            
            size_t i = 0;
            try {
                for (; i < other._size; i++) {
                    new (new_array + i) T(other._array[i]);
                }

                deallocate(_array, _size);

                // safe now
                _size = other._size;
                _capacity = other._capacity;
                _array = new_array;
            }
            catch (...) {
                deallocate(new_array, i);
                throw;
            }
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
            deallocate(_array, _size);

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