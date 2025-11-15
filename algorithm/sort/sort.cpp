#include <chrono>
#include <functional>
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <utility>
#include <vector>
#include <random>
#include <span>

// /*
// 需要额外空间做归并
// */
// template<typename T>
// std::span<T> merge_sort(std::span<T> nums) {
//     auto size = nums.size();
//     if (size == 1) {
//         return nums;
//     }
//     int mid = size / 2;
//     auto first = merge_sort(nums.subspan(0, mid));
//     auto last = merge_sort(nums.subspan(mid, size - mid));
//     int i = 0;
//     int j = 0;
//     int k = 0; 
//     std::vector<T> first_tmp(first.begin(), first.end());
//     std::vector<T> last_tmp(last.begin(), last.end());
//     while (i < first.size() && j < last.size()) {
//         while (i < first.size() && j < last.size() && first_tmp[i] <= last[j]) {
//             nums[k++] = first_tmp[i++];
//         }
//         while (i < first.size() && j < last.size() && first_tmp[i] > last[j]) {
//             nums[k++] = last_tmp[j++];
//         }
//     }
//     while (i < first.size()) {
//         nums[k++] = first_tmp[i++];
//     }
//     while (j < last.size()) {
//         nums[k++] = last_tmp[j++];
//     }
//     return nums;
// }


/*
不需要额外空间做归并
*/
template<typename T, typename Cmp=std::less_equal<T>>
std::span<T> merge_sort(std::span<T> nums) {
    auto cmp = Cmp();
    auto size = nums.size();
    if (size == 1) {
        return nums;
    }
    int mid = size / 2;
    auto first = merge_sort(nums.subspan(0, mid));
    auto last = merge_sort(nums.subspan(mid, size - mid));
    int i = 0;
    int j = 0;

    while (i < first.size() && j < last.size()) {
        while (i < first.size() && j < last.size() && cmp(first[i], last[j])) {
            i++;
        }
        while (i < first.size() && j < last.size() && !cmp(first[i], last[j])) {
            T tmp = first[i];
            first[i++] = last[j];
            int idx = j;
            while (idx + 1 < last.size() && cmp(last[idx + 1], tmp)) {
                last[idx] = last[idx + 1];
                idx++;
            }
            last[idx] = tmp;
        }
    }

    return nums;
}


std::vector<int32_t> generate_data(size_t size) {
    std::vector<int32_t> nums(size);
    // uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint64_t seed = 0;
    std::mt19937 gen(seed);
    constexpr int32_t low = 1;
    constexpr int32_t high = 100;
    std::uniform_int_distribution<> dis(low, high);
    for (auto i = 0 ; i < size; i++) {
        nums[i] = dis(gen);
    }
    return nums;
}

template<typename T>
bool check_result(std::span<T> nums) {
    if (nums.size() == 1) {
        return true;
    }
    for (auto i = 1; i < nums.size(); i++) {
        if (nums[i-1] > nums[i]) {
            std::ostringstream oss;
            oss << "nums[" << i - 1 << "]: " << nums[i - 1]
                << " but nums[" << i << "]: " << nums[i - 1]
                << "\n";
            std::cout << oss.str();
            return false;
        }
    }
    return true;
}

int main() {
    constexpr size_t size = 110;
    auto nums = generate_data(size);
    merge_sort(std::span(nums));
    if (!check_result(std::span(nums))) {
        std::cout << "sort error!\n";
    } else {
        std::cout << "pass!\n";
    }
}

