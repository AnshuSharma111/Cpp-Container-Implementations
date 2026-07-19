#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "../dynamic_array.h"

using namespace std;


// Helper to compare DynamicArray with std::vector
template<typename T>
void compare(DynamicArray<T>& a, vector<T>& b)
{
    assert(a.size() == b.size());

    for(size_t i = 0; i < b.size(); i++)
        assert(a[i] == b[i]);
}


// ===============================
// Lifetime tracking class
// ===============================

struct Tracker {

    static int constructions;
    static int destructions;

    int value;

    Tracker(int v = 0)
        : value(v)
    {
        constructions++;
    }

    Tracker(const Tracker& other)
        : value(other.value)
    {
        constructions++;
    }

    ~Tracker()
    {
        destructions++;
    }

    bool operator==(const Tracker& other) const
    {
        return value == other.value;
    }
};

int Tracker::constructions = 0;
int Tracker::destructions = 0;



int main()
{

cout << "========== Advanced DynamicArray Tests ==========\n";


// =====================================
// Empty operations
// =====================================
{
    DynamicArray<int> arr;

    assert(arr.size() == 0);
    assert(arr.empty());

    bool thrown = false;

    try {
        arr.pop_back();
    }
    catch(...)
    {
        thrown = true;
    }

    assert(thrown);

    cout << "Empty Operations Passed\n";
}



// =====================================
// Single element lifecycle
// =====================================
{
    DynamicArray<int> arr;

    arr.push_back(10);

    assert(arr.front()==10);
    assert(arr.back()==10);

    arr.pop_back();

    assert(arr.empty());

    cout << "Single Element Passed\n";
}



// =====================================
// Random push/pop stress
// =====================================
{
    DynamicArray<int> arr;
    vector<int> ref;


    for(int i=0;i<10000;i++)
    {
        arr.push_back(i);
        ref.push_back(i);
    }


    for(int i=0;i<5000;i++)
    {
        arr.pop_back();
        ref.pop_back();
    }


    compare(arr,ref);

    cout << "Push Pop Stress Passed\n";
}



// =====================================
// Reallocation integrity
// =====================================
{
    DynamicArray<string> arr;


    for(int i=0;i<10000;i++)
    {
        arr.push_back("hello_" + to_string(i));
    }


    for(int i=0;i<10000;i++)
    {
        assert(arr[i] == "hello_" + to_string(i));
    }


    cout << "String Reallocation Passed\n";
}



// =====================================
// Insert random positions
// =====================================
{
    DynamicArray<int> arr;
    vector<int> ref;


    for(int i=0;i<100;i++)
    {
        int pos = i==0 ? 0 : i/2;

        arr.insert(pos,i);
        ref.insert(ref.begin()+pos,i);
    }


    compare(arr,ref);


    cout << "Random Insert Passed\n";
}



// =====================================
// Remove random positions
// =====================================
{
    DynamicArray<int> arr;
    vector<int> ref;


    for(int i=0;i<100;i++)
    {
        arr.push_back(i);
        ref.push_back(i);
    }


    for(int i=0;i<50;i++)
    {
        int pos = i % ref.size();

        arr.remove(pos);
        ref.erase(ref.begin()+pos);
    }


    compare(arr,ref);


    cout << "Random Remove Passed\n";
}



// =====================================
// Clear after huge allocation
// =====================================
{
    DynamicArray<int> arr;


    for(int i=0;i<100000;i++)
        arr.push_back(i);


    size_t old_capacity = arr.capacity();


    arr.clear();


    assert(arr.size()==0);
    assert(arr.capacity()==old_capacity);


    cout << "Clear Capacity Preservation Passed\n";
}



// =====================================
// Reserve behaviour
// =====================================
{
    DynamicArray<int> arr;


    arr.reserve(1000);


    assert(arr.capacity() >= 1000);
    assert(arr.size()==0);


    for(int i=0;i<1000;i++)
        arr.push_back(i);


    for(int i=0;i<1000;i++)
        assert(arr[i]==i);


    cout << "Reserve Behaviour Passed\n";
}



// =====================================
// Resize growth initialization
// =====================================
{
    DynamicArray<int> arr;


    arr.resize(100);


    assert(arr.size()==100);


    for(int i=0;i<100;i++)
        assert(arr[i]==0);



    arr.resize(10);


    assert(arr.size()==10);


    cout << "Resize Growth/Shrink Passed\n";
}



// =====================================
// Copy deep copy stress
// =====================================
{
    DynamicArray<int> a;


    for(int i=0;i<10000;i++)
        a.push_back(i);



    DynamicArray<int> b=a;



    for(int i=0;i<10000;i++)
        assert(a[i]==b[i]);



    for(int i=0;i<10000;i++)
        b[i]=-1;



    for(int i=0;i<10000;i++)
        assert(a[i]==i);



    cout << "Deep Copy Stress Passed\n";
}



// =====================================
// Move leaves valid object
// =====================================
{
    DynamicArray<int> a;


    for(int i=0;i<100;i++)
        a.push_back(i);



    DynamicArray<int> b(std::move(a));


    assert(b.size()==100);



    // moved-from object should still be usable

    a.push_back(500);


    assert(a.back()==500);



    cout << "Move Validity Passed\n";
}



// =====================================
// Self move assignment
// =====================================
{
    DynamicArray<int> arr;


    for(int i=0;i<20;i++)
        arr.push_back(i);


    arr = std::move(arr);


    assert(arr.size()==20);


    for(int i=0;i<20;i++)
        assert(arr[i]==i);


    cout << "Self Move Assignment Passed\n";
}



// =====================================
// Object lifetime test
// =====================================
{
    Tracker::constructions = 0;
    Tracker::destructions = 0;


    {
        DynamicArray<Tracker> arr;


        for(int i=0;i<1000;i++)
            arr.push_back(Tracker(i));


        arr.clear();
    }


    assert(
        Tracker::constructions ==
        Tracker::destructions
    );


    cout << "Object Lifetime Passed\n";
}



// =====================================
// Bounds tests
// =====================================
{
    DynamicArray<int> arr;


    arr.push_back(1);


    bool thrown=false;


    try
    {
        arr[1];
    }
    catch(...)
    {
        thrown=true;
    }


    assert(thrown);



    cout << "Bounds Stress Passed\n";
}



cout << "\n====================================\n";
cout << "ALL ADVANCED TESTS PASSED\n";
cout << "====================================\n";


return 0;

}