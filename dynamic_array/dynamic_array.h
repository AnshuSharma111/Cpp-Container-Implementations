#pragma once

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <utility>
#include <cstddef>

template <typename T>
class DynamicArray {
private:
    size_t _size = 0;
    size_t _capacity = 0;
    T* _array = nullptr;

    // helper function to re-allocate container into new space
    // NOTE: only allocates memory and copies old elements into the new memory, does not construct in it 
    void reallocate(size_t new_capacity) {
        T* new_array = allocate(new_capacity);

        size_t i = 0;

        // new_capacity can be smaller than the number of elements we had in the array
        // in that case, preserve only the first min(_size, new_capacity) elements.
        size_t limit = std::min(_size, new_capacity);

        try {
            // if type T is move construtible OR move-only, do move
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
    
        _size = limit;
        _capacity = new_capacity;
        _array = new_array;
    }

    // double array size
    void grow() {
        size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
        reallocate(new_capacity);
    }
    
    // allocate raw memory
    static T* allocate(size_t sz) {
        if (sz == 0) return nullptr;
        T* new_array = static_cast<T*>(::operator new(sz * sizeof(T), std::align_val_t(alignof(T))));
        return new_array;
    }

    // destruct objects at memory
    // destructs objects in the index range [0, i)
    static void destruct(T* arr, size_t i) {
        if (!arr) return;

        for (; i > 0; i--) {
            arr[i - 1].~T();
        }
    }

    // destruct objects at memory
    // destructs objects in the index range [0, i)
    // void destruct(size_t i) {
    //     if (!_array) return;

    //     for (; i > 0; i--) {
    //         _array[i - 1].~T();
    //     }
    // }

    // deallocate memory
    static void deallocate(T* arr) {
        if (arr == nullptr) return;
        ::operator delete(arr, std::align_val_t(alignof(T)));
    }

    // assign default objects to container memory
    // constructs objects in the index range [0, i)
    static void default_construct(T* arr, size_t i) {
        if (!arr) return;

        size_t cur = i;

        try {
            for (; cur > 0; cur--) {
                ::new (static_cast<void*>(arr + cur - 1)) T();
            }
        } catch (...) {
            // we failed at index cur therefore cur has not been constructed to, therefore destruct from [cur, i)
            for (; cur < i; cur++) {
                arr[cur].~T();
            }

            throw;
        }
    }

public:
    // Design Note 1 : The container NEEDS to have a destructor
    // That is the conntract but it's nice to be explicit
    static_assert(std::is_destructible_v<T>, "Type needs to be destructible!"); 

    // Design Note 2 : I allow types with destructors not marked as nothrow
    // But the contract of the container is that the destructor is nothrow. Therefore, I do not guard against a throwing destructor
    // as that is a violation of that contract

    // Design Note 3 : I allow T to not be copy constructible as long as the user does not call copy constructing methods such as
    // copy constructor or assignment. This is for the sake of move-only types like unique_ptr
    // same with copy-only types


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

    // forwarding reference push back
    template <typename U>
    requires std::constructible_from<T, U&&>
    void push_back(U&& x) {
        if (_size == _capacity) {
            // reallocation invalidates previous references therefore anti-aliasing first
            T temp(std::forward<U>(x));
            grow();
            new (_array + _size) T(std::move(temp));
        } else {
            new (_array + _size) T(std::forward<U>(x));
        }

        _size++;
    }

    // forwarding reference insert
    template <typename U>
    // The function accepts anything
    // as long as it is constructible into T
    // therefore the constraint
    requires std::constructible_from<T, U&&> && std::is_move_assignable_v<T>
    void insert(size_t idx, U&& x) {
        if (idx > _size) throw std::out_of_range("index out of range!");
        // STRATEGY: If container needs to be resized, we can ensure strong exception safety guarantee
        if (_size == _capacity) {
            size_t new_capacity = (_capacity == 0) ? 1 : _capacity * 2;
            T* new_array = allocate(new_capacity);
            size_t i = 0, j = 0;

            
            try {
                // prevent aliasing issue
                // throw spot 1 : construction of temp fails
                // what happens on failure ? catch block deallocates and destructs new_array, no harm
                T temp(std::forward<U>(x));

                // before idx
                for (; i < idx; i++, j++) {
                    // throw spot 2
                    // A. Move is not noexcept therefore copy is called and copying fails
                    // B. Construction of T fails
                    // In either case, we have placed i - 1 elements and would like to destroy all of them and deallocate new_array. That is done by the catch block
                    new (new_array + i) T(std::move_if_noexcept(_array[j]));
                }

                // at idx
                // throw spot 3
                // Construction of T fails
                // we need to destruct all i - 1 elements constructed
                // that is done by the catch block
                new (new_array + idx) T(std::move(temp));
                i++;

                // after idx
                // same as throw spot 2
                for (; j < _size; ++i, ++j) {
                    new (new_array + i) T(std::move_if_noexcept(_array[j]));
                }
            } catch (...) {
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
            // NOTE: This part is not up to the mark because reallocation path uses move construction 
            // but this branch uses move assignment which is strictly worse
            // so if a user tries to initialise for a type where move throws, then code will not compiler
            // this differs from std::vector, where the code will compile but will throw at runtime

            // prevent aliasing issues
            T temp(std::forward<U>(x));

            // if _size == 0, we access array[size - 1] which is UB
            // so just call push_back then
            if (_size == 0) {
                push_back(std::move(temp));
                return;
            }

            // construct at _size
            // throw spot 4
            // construction of T fails
            // no problem, we haven't modified _array yet
            new (_array + _size) T(std::move_if_noexcept(_array[_size - 1]));

            size_t num_modified = 1; // we already constructed one at _size
            size_t failure_index = _size - 1; // start with largest possible value

            try {
                // throw spot 5
                // A. construction of T fails
                // B. placement new fails
                // If we fail here, we destruct all constructed elements so far
                // Having a consistent unspecified state is better than leaving the container half constructed or built
                // so that we can avoid double destruction or construction in the future
                for (size_t i = _size - 1; i > idx; --i) {
                    // in case we fail, truncate the container upto the failed index
                    failure_index = i;

                    _array[i] = std::move(_array[i - 1]);
                    num_modified++;
                }
            }
            catch (...) {
                // destroy all elements from the first element in unspecified state (_array[i - 1])
                // to the end to remove the tainted tail
                for (size_t k = 0; k < num_modified; ++k) {
                    _array[_size - k].~T();
                }

                // update size
                _size = failure_index;

                // now we have discarded all elements from i - 1 to _size
                throw;
            }

            // assign into idx
            // throw spot 6
            // same as spot 4
            _array[idx] = std::move(temp);
            _size++;
        }
    }

    // pop element from the end of the container
    T pop_back() {
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

    // resize container
    // behavior: if resize to less than _size, destruct elements forward and adjust _size
    //           if resize to more than _size, initialise empty slots with new elements
    //           if resize to more _capacity, then increase container size
    void resize (size_t sz) {
        if (sz < _size) {
            // _size is the last non-constructed index, therefore we need to destruct [sz, _size)
            // case 1 : sz = _size - 1 => _size - sz = 1; 1 element destructed, which is intended behavior
            // case 2 : sz = 0 => _size - sz = _size; empty the array, intended behavior

            // static destruct will not update _size
            destruct(_array + sz, _size - sz);
            _size = sz;
        } else {
            if (sz > _capacity) {
                // allocate new buffer of sz and reallocate _arr into it
                // non-static private reallocate() will change _size = sz
                reallocate(sz);
            }

            // it lies between _size and _capacity
            // construct in [_size, sz)

            // static default_construct will not update _size
            default_construct(_array + _size, sz - _size);
            _size = sz;
        }
    }

    // clear function to empty the container
    void clear () noexcept {
        destruct(_array, _size);
        _size = 0;   
    }

    // indexing access
    T& operator[](size_t idx) {
        if (idx >= _size)
            throw std::out_of_range("index out of range");
        return _array[idx];
    }

    const T& operator[](size_t idx) const {
        if (idx >= _size)
            throw std::out_of_range("index out of range");
        return _array[idx];
    }
    // size & capacity getter
    size_t size() const noexcept { return _size; }
    size_t capacity() const noexcept { return _capacity; }

    // iterator support
    const T* begin() const { return _array; }
    const T* end() const { return _array + _size ; }

    // the first and the last element
    // NOTE : calling on empty container is undefined behavior
    const T& front() const { return *_array; }
    const T& back() const { return *(_array + _size - 1); }

    bool empty() { return _size == 0; }
};