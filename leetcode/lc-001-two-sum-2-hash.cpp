// 18/04/23 = Mon

// 1. Two Sum

// Given an array of integers, return indices of the two numbers such that they add up to a specific target.

// You may assume that each input would have exactly one solution, and you may not use the same element twice.

// Example:

// Given nums = [2, 7, 11, 15], target = 9,

// Because nums[0] + nums[1] = 2 + 7 = 9,
// return [0, 1].

// [Idea]

// Use hash table to do 1-pass scan of the array.

// For any number in the iteration, check if we have seen its complement.

// [Complexity]

// O(1) (for hash table)  * n = O(n)

// [Comparison]

// Much shorter, and faster than using 2 pointers, which takes O(n log (n)).

// To run: a

// To run with seed 100: a 100

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

using namespace std;

class Solution
{
public:
    vector<int> twoSum(vector<int> & nums, int target) 
    {
        unordered_map<int, int> past;
        for (int c, i = 0; i != nums.size(); ++i) {
            c = target - nums[i];
            auto it = past.find(c);
            if (it != past.end())
                return {it->second, i};
            past[nums[i]] = i;
        }
        return {};
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