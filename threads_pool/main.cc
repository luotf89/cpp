//main.cc
#include <vector>
#include <thread>
#include <iostream>
#include "block_queue.h"
#include "threads_pool.h"
#include <unistd.h>
#include <cstdlib>
#include <random>
#include <cassert>



#if defined (__i386__)
static __inline__ unsigned long long GetCycleCount(void)
{
        unsigned long long int x;
        __asm__ volatile("rdtsc":"=A"(x));
        return x;
}
#elif defined (__x86_64__)
static __inline__ unsigned long long GetCycleCount(void)
{
        unsigned hi,lo;
        __asm__ volatile("rdtsc":"=a"(lo),"=d"(hi));
        return ((unsigned long long)lo)|(((unsigned long long)hi)<<32);
}
#endif


enum class RandomDistribution:std::int32_t{
    RNG_UNIFORM = 0,
    RNG_NORMAL
};


template<typename T>
void random_op_impl(T param0, T param1, T* ret, int distribution_mode, 
                    std::mt19937* gen1, int offset, int size) {
  std::mt19937 gen = *gen1;
  if (distribution_mode == static_cast<int>(RandomDistribution::RNG_UNIFORM)) {
    // only support float, double, or long double.
    std::uniform_real_distribution<T> d(param0, param1);
    for (int i = 0; i < size; i++) {
    //   std::cout << "value: " << d(gen) << std::endl;
      auto res = d(gen);
      assert(res > param0 && res < param1);
      ret[offset+i] = res;
    }
  } else if (distribution_mode == static_cast<int>(RandomDistribution::RNG_NORMAL)) {
    // only support float, double, or long double.
    std::normal_distribution<T> d(param0, param1);
    for (int i = 0; i < size; i++) {
      auto res = d(gen);
      assert(res > param0 && res < param1);
      ret[offset+i] = res;
    }

  } else {
    std::cout << "Unsupported RandomDistribution Type: "
                   << distribution_mode << "\n";
  }
}


void testcase0() {
    float low = -1.5;
    float high = 1;
    int distribution_mode = static_cast<int>(RandomDistribution::RNG_UNIFORM);
    int elem_num = 1024*1024;
    float* ret1 = new float [elem_num]();
    float* ret2 = new float [elem_num]();
    int seed = 0;
    unsigned long long t1, t2, t3, t4;
    int offset = 0;
    int size = elem_num;
     int loop_times = 1;

    static uint64_t curr_seed = seed;
    std::mt19937 gen(curr_seed);
    curr_seed++;

    t1 = GetCycleCount();
    for (int loop = 0; loop < loop_times; loop++) {
    random_op_impl(low, high, ret1, distribution_mode, &gen, offset, size);
    }
    t2 = GetCycleCount();
    std::cout << "single thread time: " << (t2-t1) /1804000000.0 << std::endl;

    constexpr unsigned int elem_num_per_thread = 1024;
    ThreadsPool* threads_pool = Singleton<ThreadsPool>::get_instance();
    // std::cout << "current thread pool addr: " << threads_pool << std::endl;
    // std::vector<int> offsets(thread_num, 0);
    // std::vector<int> sizes(thread_num, 0);
    // int raw_size = elem_num / thread_num;
    // int raw_rem = elem_num % thread_num;
    // std::cout << "current thread_num: " << thread_num << std::endl;
    int package_num =  (elem_num + elem_num_per_thread - 1) / elem_num_per_thread;
    std::vector<std::future<void>> rets;
    for (int loop = 0; loop < loop_times; loop++) {
    for (int i = 0; i < package_num - 1; i++) {
        rets.push_back(threads_pool->submit(random_op_impl<float>, low, high, ret2, distribution_mode, &gen, i*elem_num_per_thread, elem_num_per_thread));
    }
    int rem_size = elem_num - (package_num - 1) * elem_num_per_thread;
    rets.push_back(threads_pool->submit(random_op_impl<float>, low, high, ret2, distribution_mode, &gen, (package_num - 1)*elem_num_per_thread, rem_size));
    }
    t3 = GetCycleCount();
    for (int i = 0; i < rets.size(); i++) {
        rets[i].get();
    }
    t4 = GetCycleCount();
    std::cout << "multi thread time: " << (t4-t3) /1804000000.0 << std::endl;
    // for(int i = 0; i < elem_num / 12; i ++) {
    //     for (int j = 0; j < 12; j++) {
    //         std::cout << ret2[i*12+j] << "  ";
    //     }
    //     std::cout << std::endl;
    // }
    delete [] ret1;
    delete [] ret2;
}
    

int main() {
    testcase0();

    return 0;
}