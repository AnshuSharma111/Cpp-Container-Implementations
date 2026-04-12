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
        T* new_array = static_cast<T*>(::operator new(new_capacity * sizeof(T), std::align_val_t(alignof(T))));

        size_t i = 0;
        try {
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                for (; i < _size; i++) {
                    new (new_array + i) T(std::move(_array[i])); // placement new to construct object
                }
            }
            else {
                for (; i < _size; i++) {
                    new (new_array + i) T(_array[i]); // invoke copy constructor
                }
            }
        }
        catch (...) {
            // rollback in case of failure
            deallocate(new_array, i);
            throw;
        }

        deallocate(_array, _size);
        _capacity = new_capacity;
        _array = new_array;
    }

    // double array size
    void grow() {
        size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        resize(new_capacity);
    }

    // half array size
    void shrink() {
        size_t new_capacity = std::max(_size, _capacity / 2);
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
    ~DynamicArray() noexcept { // if ~T throws, not my problem
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

    // push by const lvalue reference
    void push(const T& x) {
        if (_size == _capacity) grow();
        new (_array + _size) T(x);
        _size++;
    }

    // push by rvalue binding
    void push(T&& x) {
        if (_size == _capacity) grow();
        new (_array + _size) T(std::move(x));
        _size++;
    }

    template <typename U>
    void insert(size_t idx, U&& x) {
        if (idx > _size) throw std::out_of_range("index out of range!");
        if (_size == _capacity) grow();

        // insertion at end
        if (idx == _size) {
            new (_array + _size) T(std::forward<U>(x));
            _size++;
            return;
        }

        // if we can move, shift in-place
        if constexpr (std::is_nothrow_move_constructible_v<T> &&
            std::is_nothrow_move_assignable_v<T>) {
            // may throw but container remains intact
            T temp(std::forward<U>(x));

            // move last element into uninitialised slot
            new (_array + _size) T(std::move(_array[_size - 1]));

            for (size_t i = _size - 1; i > idx; --i) {
                _array[i] = std::move(_array[i - 1]);
            }

            // place the new value
            _array[idx] = std::move(temp);

            ++_size;
        }
        else {
            // If moves can throw, allocate a new buffer to maintain the strong exception guarantee.
            // acquire new memory
            T* new_array = static_cast<T*>(::operator new((_size + 1) * sizeof(T), std::align_val_t(alignof(T))));
            size_t i = 0;

            try {
                // before idx
                for (; i < idx; ++i) {
                    new (new_array + i) T(std::move_if_noexcept(_array[i]));
                }

                // inserted element
                new (new_array + idx) T(std::forward<U>(x));
                ++i;

                // after idx
                for (size_t j = idx; j < _size; ++j, ++i) {
                    new (new_array + i) T(std::move_if_noexcept(_array[j]));
                }
            }
            catch (...) {
                deallocate(new_array, i);
                throw;
            }

            // commit
            deallocate(_array, _size);
            _array = new_array;
            _size++;
        }
    }

    // pop element from the end of the container
    T pop() {
        if (_size == 0) throw std::runtime_error("array is empty!");

        _size--;
        T ret_val = std::move(_array[_size]);
        _array[_size].~T();
        if (_size < _capacity / 4) shrink();
        return ret_val;
    }

    // remove element by index
    void remove(size_t idx) {
        if (idx >= _size) throw std::invalid_argument("index out of range!");

        for (size_t i = idx + 1; i < _size; i++) {
            _array[i - 1] = std::move(_array[i]);
        }

        _size--;
        _array[_size].~T();
    }

    // indexing access
    T& operator[](size_t idx) { return _array[idx]; }
    const T& operator[](size_t idx) const { return _array[idx]; }

    // size & capacity getter
    size_t size() const noexcept { return _size; }
    size_t capacity() const noexcept { return _capacity; }
};