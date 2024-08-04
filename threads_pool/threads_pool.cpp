#include "threads_pool.h"
#include "singleton.h"

static int __register_thread_pool = []() {
  constexpr int thread_num = 8;
  ThreadsPool *tmp = Singleton<ThreadsPool>::get_instance(thread_num);
  return 0;
}();
