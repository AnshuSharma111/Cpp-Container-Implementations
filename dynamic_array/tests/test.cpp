#include <iostream>
#include <cassert>
#include "../dynamic_array.h"

using namespace std;

int main() {

    cout << "========== DynamicArray Tests ==========\n";

    // =====================================
    // Constructor
    // =====================================
    {
        DynamicArray<int> arr;

        assert(arr.size() == 0);
        assert(arr.empty());

        cout << "Constructor Test Passed\n";
    }

    // =====================================
    // push_back
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(10);
        arr.push_back(20);
        arr.push_back(30);

        assert(arr.size() == 3);
        assert(arr[0] == 10);
        assert(arr[1] == 20);
        assert(arr[2] == 30);

        cout << "push_back Test Passed\n";
    }

    // =====================================
    // Massive Reallocation
    // =====================================
    {
        DynamicArray<int> arr;

        for (int i = 0; i < 50000; i++)
            arr.push_back(i);

        assert(arr.size() == 50000);

        for (int i = 0; i < 50000; i++)
            assert(arr[i] == i);

        cout << "Reallocation Test Passed\n";
    }

    // =====================================
    // operator[]
    // =====================================
    {
        DynamicArray<int> arr;

        for (int i = 0; i < 10; i++)
            arr.push_back(i);

        arr[5] = 100;

        assert(arr[5] == 100);

        cout << "operator[] Test Passed\n";
    }

    // =====================================
    // Bounds Checking
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(1);

        bool thrown = false;

        try {
            arr[10];
        }
        catch (...) {
            thrown = true;
        }

        assert(thrown);

        cout << "Bounds Test Passed\n";
    }

    // =====================================
    // front()
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(50);
        arr.push_back(60);

        assert(arr.front() == 50);

        cout << "front() Test Passed\n";
    }

    // =====================================
    // back()
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(50);
        arr.push_back(60);

        assert(arr.back() == 60);

        cout << "back() Test Passed\n";
    }

    // =====================================
    // pop_back()
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);

        arr.pop_back();

        assert(arr.size() == 2);
        assert(arr.back() == 2);

        cout << "pop_back Test Passed\n";
    }

    // =====================================
    // insert(begin)
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(2);
        arr.push_back(3);

        arr.insert(0,1);

        assert(arr[0] == 1);
        assert(arr[1] == 2);
        assert(arr[2] == 3);

        cout << "Insert Beginning Passed\n";
    }

    // =====================================
    // insert(middle)
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(1);
        arr.push_back(3);

        arr.insert(1,2);

        assert(arr[0] == 1);
        assert(arr[1] == 2);
        assert(arr[2] == 3);

        cout << "Insert Middle Passed\n";
    }

    // =====================================
    // insert(end)
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(1);
        arr.push_back(2);

        arr.insert(2,3);

        assert(arr.back() == 3);

        cout << "Insert End Passed\n";
    }

    // =====================================
    // remove(begin)
    // =====================================
    {
        DynamicArray<int> arr;

        for(int i=1;i<=5;i++)
            arr.push_back(i);

        arr.remove(0);

        assert(arr.size()==4);
        assert(arr[0]==2);

        cout << "Erase Beginning Passed\n";
    }

    // =====================================
    // remove(middle)
    // =====================================
    {
        DynamicArray<int> arr;

        for(int i=1;i<=5;i++)
            arr.push_back(i);

        arr.remove(2);

        assert(arr.size()==4);

        assert(arr[0]==1);
        assert(arr[1]==2);
        assert(arr[2]==4);
        assert(arr[3]==5);

        cout << "Erase Middle Passed\n";
    }

    // =====================================
    // remove(end)
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(1);
        arr.push_back(2);
        arr.push_back(3);

        arr.remove(2);

        assert(arr.size()==2);
        assert(arr.back()==2);

        cout << "Erase End Passed\n";
    }

    // =====================================
    // clear()
    // =====================================
    {
        DynamicArray<int> arr;

        for(int i=0;i<100;i++)
            arr.push_back(i);

        arr.clear();

        assert(arr.size()==0);
        assert(arr.empty());

        cout << "Clear Test Passed\n";
    }

    // =====================================
    // resize()
    // =====================================
    {
        DynamicArray<int> arr;

        arr.resize(10);

        assert(arr.size()==10);

        arr.resize(5);

        assert(arr.size()==5);

        cout << "Resize Test Passed\n";
    }

    // =====================================
    // reserve()
    // =====================================
    // {
    //     DynamicArray<int> arr;

    //     arr.reserve(100);

    //     assert(arr.capacity() >= 100);

    //     cout << "Reserve Test Passed\n";
    // }

    // =====================================
    // Copy Constructor
    // =====================================
    {
        DynamicArray<int> arr;

        for(int i=0;i<10;i++)
            arr.push_back(i);

        DynamicArray<int> copy(arr);

        copy[0]=100;

        assert(arr[0]==0);
        assert(copy[0]==100);

        cout << "Copy Constructor Passed\n";
    }

    // =====================================
    // Copy Assignment
    // =====================================
    {
        DynamicArray<int> arr;

        for(int i=0;i<10;i++)
            arr.push_back(i);

        DynamicArray<int> copy;

        copy = arr;

        copy[5]=999;

        assert(arr[5]==5);
        assert(copy[5]==999);

        cout << "Copy Assignment Passed\n";
    }

    // =====================================
    // Move Constructor
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(1);
        arr.push_back(2);

        DynamicArray<int> moved(std::move(arr));

        assert(moved.size()==2);

        cout << "Move Constructor Passed\n";
    }

    // =====================================
    // Move Assignment
    // =====================================
    {
        DynamicArray<int> arr;

        arr.push_back(5);
        arr.push_back(6);

        DynamicArray<int> other;

        other = std::move(arr);

        assert(other.size()==2);

        cout << "Move Assignment Passed\n";
    }

    // =====================================
    // Self Assignment
    // =====================================
    {
        DynamicArray<int> arr;

        for(int i=0;i<10;i++)
            arr.push_back(i);

        arr = arr;

        for(int i=0;i<10;i++)
            assert(arr[i]==i);

        cout << "Self Assignment Passed\n";
    }

    // =====================================
    // Stress Test
    // =====================================
    // {
    //     DynamicArray<int> arr;

    //     for(int i=0;i<100000;i++)
    //         arr.push_back(i);

    //     for(int i=0;i<50000;i++)
    //         arr.pop_back();

    //     for(int i=0;i<25000;i++)
    //         arr.erase(0);

    //     assert(arr.size()==25000);

    //     cout << "Stress Test Passed\n";
    // }

    cout << "\n====================================\n";
    cout << "ALL TESTS PASSED\n";
    cout << "====================================\n";

    return 0;
}