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
        T* new_array = allocate(new_capacity);

        size_t i = 0;
        size_t limit = std::min(_size, new_capacity);

        try {
            // if type is move construtible OR move-only, move
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                for (; i < limit; i++) {
                    new (new_array + i) T(std::move(_array[i]));
                }
            }
            else {
                for (; i < limit; i++) {
                    new (new_array + i) T(_array[i]); // invoke copy constructor
                }
            }
        }
        catch (...) {
            // rollback in case of failure
            destruct(new_array, i);
            deallocate(new_array);
            throw;
        }

        destruct(_array, _size);
        deallocate(_array);
        _capacity = new_capacity;
        _array = new_array;
    }

    // double array size
    void grow() {
        size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        resize(new_capacity);
    }
    
    // allocate raw memory
    static T* allocate(size_t sz) {
        if (sz == 0) return nullptr;
        T* new_array = static_cast<T*>(::operator new(sz * sizeof(T), std::align_val_t(alignof(T))));
        return new_array;
    }
    
    // destruct objects at memory
    static void destruct(T* arr, size_t i) {
        if (!arr) return;

        for (; i > 0; i--) {
            arr[i - 1].~T();
        }
    }

    // deallocate memory
    static void deallocate(T* arr) {
        if (arr == nullptr) return;
        ::operator delete(arr, std::align_val_t(alignof(T)));
    }

public:
    // Design Note: not designed to handle throwing destructors
    static_assert(std::is_nothrow_destructible_v<T>, "type is not nothrow destructible");
    // Design Note : T must be copy constructible otherwise copy constructor and assignment don't work
    static_assert(std::is_copy_constructible_v<T>, "type is not copy constructible!");

    // default constructor
    DynamicArray() noexcept = default;

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
        _array = allocate(_capacity);
    }

    // fill constructor
    DynamicArray(size_t capacity, const T& x) { // const T& binds to both l and r values
        if (capacity == 0) {
            _array = nullptr;
            _size = 0;
            _capacity = 0;
            return;
        }

        // accquire memory
        _array = allocate(capacity);

        // now construct
        size_t i = 0;
        try {
            for (; i < capacity; i++) {
                new (_array + i) T(x);
            }

            // safely constructed, update parameters
            _capacity = capacity;
            _size = capacity;
        }
        catch (...) {
            destruct(_array, i);
            deallocate(_array);
            throw;
        }
    }

    // destructor
    ~DynamicArray() noexcept {
        // Design Note : Guaranteed to not throw because of assertion T is nothrow destructible
        destruct(_array, _size);
        deallocate(_array);
    }

    // copy constructor
    DynamicArray(const DynamicArray& other) 
        : _array(nullptr), _size(0), _capacity(0)
    {
        // accquire memory
        _array = allocate(other._capacity);
        _capacity = other._capacity;

        // try constructing objects
        size_t i = 0;
        try {
            for (; i < other._size; i++) {
                // Scenario : copy constructor throws, the catch block exceutes and undoes progress.
                new (_array + i) T(other._array[i]);
            }

            // safely copied
            _size = other._size;
        }
        catch (...) {
            destruct(_array, i);
            deallocate(_array);
            throw;
        }
    }

    // assignment operator
    DynamicArray& operator=(const DynamicArray& other) {
        // A = B (copy B into A)
        if (this != &other) {
            T* new_array = allocate(other._capacity);

            size_t i = 0;
            try {
                for (; i < other._size; i++) {
                    // Scenario : copy constructor fails, catch block undoes progress.
                    new (new_array + i) T(other._array[i]);
                }

                destruct(_array, _size);
                deallocate(_array);

                // safe now
                _size = other._size;
                _capacity = other._capacity;
                _array = new_array;
            }
            catch (...) {
                destruct(new_array, i);
                deallocate(new_array);
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
            destruct(_array, _size);
            deallocate(_array);

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

    // forwarding reference insert
    template <typename U>
    void insert(size_t idx, U&& x) {
        if (idx > _size) throw std::out_of_range("index out of range!");
        // STRATEGY: If container needs to be resized, we can ensure strong exception safety guarantee

        if (_size == _capacity) {
            size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
            T* new_array = allocate(new_capacity);
            size_t i = 0, j = 0;

            // prevent aliasing issue
            T temp(std::forward<U>(x));

            try {
                // before idx
                for (; i < idx; i++, j++) {
                    new (new_array + i) T(std::move_if_noexcept(_array[j]));
                }

                // at idx
                new (new_array + idx) T(std::move(temp));
                i++;

                // after idx
                for (; j < _size; ++i, ++j) {
                    new (new_array + i) T(std::move_if_noexcept(_array[j]));
                }
            } catch (...) {
                // alright, something went wrong. Pack it up
                destruct(new_array, i);
                deallocate(new_array);
                throw;
            }

            // we can commit now
            destruct(_array, _size);
            deallocate(_array);

            _array = new_array;
            _capacity = new_capacity;
            _size++;
        } 
        // otherwise we try to shift elements and the exception safety guarantee degrades to basic
        else {
            // prevent aliasing issues
            T temp(std::forward<U>(x));

            // if _size == 0, we access array[size - 1] which is UB
            if (_size == 0) {
                new (_array) T(std::move(temp));
                _size++;
                return;
            }

            // construct at _size
            new (_array + _size) T(std::move(_array[_size - 1]));

            // shift backward
            for (size_t i = _size - 1; i > idx; --i) {
                _array[i] = std::move(_array[i - 1]);
            }

            // assign into idx
            _array[idx] = std::move(temp);
            _size++;
        }
    }

    // pop element from the end of the container
    T pop() {
        if (_size == 0) throw std::runtime_error("array is empty!");

        _size--;
        T ret_val = std::move(_array[_size]);
        _array[_size].~T();
        return ret_val;
    }

    // remove element by index
    void remove(size_t idx) {
        if (idx >= _size) throw std::out_of_range("index out of range!");

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