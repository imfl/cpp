// 18/04/23 = Mon

// 1. Two Sum

// Given an array of integers, return indices of the two numbers such that they add up to a specific target.

// You may assume that each input would have exactly one solution, and you may not use the same element twice.

// Example:

// Given nums = [2, 7, 11, 15], target = 9,

// Because nums[0] + nums[1] = 2 + 7 = 9,
// return [0, 1].

// [Idea]

// On a sorted array, use 2 pointers to go from middle to two ends.

// 0. sort the array;

// 1. establish a separation line, so that it cannot be the case that both numbers are on the same side;

// 2. use two pointers: left pointer goes left and right pointer goes right;

// 3. not found: left pointer is at the left end, but still overestimates, or right pointer is at the right end, but still underestimates.

// [Complexity]

// O(n log(n)) (for sorting) + O(n) (for searching) = O(n log(n))

// To run: a

// To run with seed 100: a 100

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <set>
#include <vector>

using namespace std;

class Solution
{
public:
    vector<int> twoSum(vector<int> & nums, int target) 
    {
        assert(nums.size() > 1);
        vector<array<int, 2>> w;
        int k = 0;
        for (auto e : nums)
            w.push_back(array<int, 2>{e, k++});

        sort(w.begin(), w.end(), [](array<int, 2> a1, array<int, 2> a2) -> bool
                                 { return a1[0] < a2[0]; });

        const int N = nums.size();

        int i = 0, j = 1;
        while (j != N && w[i][0] + w[j][0] < target) {
            ++i;
            ++j;
        }
        bool not_found = false;
        for (int sum = w[i][0] + w[j][0];
                 sum != target;
                 sum = w[i][0] + w[j][0]) {
            // overestimate
            if (sum > target) {
                if (i != 0)
                    i -= 1;
                else {
                    not_found = true;
                    break;
                }
            }
            // underestimate
            else {
                if (j != N - 1)
                    j += 1;
                else {
                    not_found = true;
                    break;
                }
            }
        }
        return not_found ? vector<int>{} : vector<int> {w[i][1], w[j][1]};
    }
};

void process(vector<int> & v, int t)
{
    auto ans = Solution().twoSum(v, t);
    if (!ans.empty())
        printf("%2d = a[%d] + a[%d] = %2d + %2d\n", t, ans[0], ans[1], v[ans[0]], v[ans[1]]);
}

void print(const vector<int> & v)
{
    for (int i = 0; i != v.size(); ++i)
        printf("[%d] %2d    ", i, v[i]);
    putchar('\n');
}

int main(int argc, char * argv[])
{
    unsigned long seed;
    if (argc != 1)
        seed = atol(argv[1]);
    else
        seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine e(seed);
    uniform_int_distribution<int> u(0, 100);

    vector<int> v;
    set<int> s;
    for (int d, i= 0; i != 10; ++i) {
        do
            d = u(e);
        while (s.find(d) != s.end());
        s.insert(d);
        v.push_back(d);
    }

    cout << endl;
    print(v);
    cout << endl;
    for (int i = *min_element(v.cbegin(), v.cend());
             i != *max_element(v.cbegin(), v.cend()); ++i)
        process(v, i);
    return 0;
}