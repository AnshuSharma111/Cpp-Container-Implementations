#include <iostream>
#include "dynamic_array.h";
#include <string>
using namespace std;

int main() {
	DynamicArray<string> arr;

	arr.push("J Cole");
	arr.push("Kendrick Lamar");
	arr.push("Future");
	arr.push("JID");

	for (auto x : arr) {
		cout << x << ' ';
	}

	cout << endl;

	arr.insert("21 Savage", 2);	
	try {
		arr.insert("Drake", 10);
	}
	catch (...) {
		cout << "We erred" << '\n';
	}

	for (auto x : arr) {
		cout << x << ' ';
	}
	cout << endl;

	arr.pop();
	arr.pop();
	arr.pop();

	for (auto x : arr) {
		cout << x << ' ';
	}
	cout << endl;

	DynamicArray<string> arr2(arr);
	arr2.push("Baby Keem");
	for (auto x : arr2) {
		cout << x << ' ';
	}
	cout << endl;
	for (auto x : arr) {
		cout << x << ' ';
	}
	cout << endl;

	arr = move(arr2);
	for (auto x : arr) {
		cout << x << ' ';
	}
	cout << endl;
}