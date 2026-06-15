#include "dynamic_array.h"
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

class faulty {
private:
    int data;
    string name;
public:
    faulty(int x, string y) {
        data = x;
        name = y;
    }

    faulty(faulty&& X) {
        throw std::runtime_error("move failed");
    }

    friend ostream& operator<<(ostream& os, const faulty& f);

};

ostream& operator<<(ostream& os, const faulty& f) {
    os << f.data << " " << f.name << '\n';
    return os;
}

int main() {
    DynamicArray<faulty> arr;

    arr.push_back(faulty(1, "Anshu"));
    arr.push_back(faulty(2, "Priyanka"));
    arr.push_back(faulty(3, "Hardik"));
    arr.push_back(faulty(4, "Mumma"));
    arr.insert(2, faulty(5, "Papa"));

    cout << arr[0] << '\n';
    cout << arr[1] << '\n';
    cout << arr[2] << '\n';
    cout << arr[3] << '\n';
    cout << arr[4] << '\n';

    // vector<faulty> vec;

    // vec.push_back(faulty(1, "Anshu"));
    // vec.push_back(faulty(2, "Priyanka"));
    // vec.push_back(faulty(3, "Hardik"));
    // vec.push_back(faulty(4, "Mumma"));

    // for (faulty& x : vec) cout << x;
}