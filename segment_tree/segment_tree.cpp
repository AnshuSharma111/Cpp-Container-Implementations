#include <iostream>
#include <vector>
#include <algorithm>

#define LOG(x) std::cout << x << '\n';

class SegmentTree {
private:
	std::vector<int> tree;

public:
	SegmentTree (const std::vector<int>& arr) {
		int n = arr.size();
		tree.resize(2 * n);

		std::copy(arr.begin(), arr.end(), tree.begin() + n);
		for (int i = n - 1; i > 0; i--) {
			tree[i] = std::min(tree[2 * i], tree[2 * i + 1]);
		}
	}

	int find_min(int l, int r) {
		int n = tree.size() / 2;
		l += n;
		r += n;

		int ans = INT_MAX;
		while (l <= r) {
			if (l % 2 == 1) {
				ans = std::min(ans, tree[l]);
				l++;
			}
			if (r % 2 == 0) {
				ans = std::min(ans, tree[r]);
				r--;
			}
			l /= 2;
			r /= 2;
		}
		return ans;
	}

	void update(int idx, int new_val) {
		int n = tree.size() / 2;
		idx += n;

		tree[idx] = new_val;
		while (idx > 1) {
			idx /= 2;
			tree[idx] = std::min(tree[idx * 2], tree[idx * 2 + 1]);
		}
	}
};

int main() {
	std::vector<int> arr = {12, -1, 3, 5, 7, 9, 12, 34, 29, 56, 0};
	int n = 1;

	while (n < arr.size()) {
		n *= 2;
	}

	arr.resize(n);

	SegmentTree st(arr);

	LOG(st.find_min(1, 5));
	LOG(st.find_min(2, 7));
	LOG(st.find_min(0, 10));
	LOG(st.find_min(9, 10));
	LOG(st.find_min(4, 8));

	st.update(1, 5);

	LOG(st.find_min(1, 5));
	LOG(st.find_min(2, 7));
	LOG(st.find_min(0, 10));
	LOG(st.find_min(9, 10));

	return 0;
}