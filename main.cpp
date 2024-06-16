#include "ThreadPool.h"
#include <iostream>
#include <unistd.h>

void *thread_helper(void *arg) {
  std::cout << " Hi from thread helper" << std::endl;
  return nullptr;
}

class Testing {
public:
  struct ThreadPoolHelper {
    Testing *test;
    int to_run;
    ThreadPoolHelper(Testing *test_, int to_run_)
        : test(test_), to_run(to_run_) {}
  };

  Testing(ThreadPool *th_, int k) : th(th_) {
    t = new ThreadPoolHelper(this, k);
    AddTaskToQueue();
  };
  Testing(Testing &&) = default;
  Testing(const Testing &) = default;
  Testing &operator=(Testing &&) = default;
  Testing &operator=(const Testing &) = default;
  ~Testing() { delete t; };

  inline void AddTaskToQueue() noexcept { th->EnqueTask(&Testing::run, t); }

private:
  inline void run_helper(int i) noexcept {
    switch (i) {
    case 0:
      std::cout << "Hi from zero" << std::endl;
      break;
    case 1:
      std::cout << "Hi from one" << std::endl;
      break;
    case 2:
      std::cout << "Hi from two" << std::endl;
      break;
    case 3:
      std::cout << "Hi from three" << std::endl;
      break;
    default:
      std::cout << "Hi given wrong number" << std::endl;
      break;
    }

    sleep(1);
  }
  inline static void *run(void *arg) noexcept {
    auto tph = static_cast<ThreadPoolHelper *>(arg);
    tph->test->run_helper(tph->to_run);
    return nullptr;
  }
  ThreadPool *th;
  ThreadPoolHelper *t;
};

int main(int argc, char *argv[]) {
  int *cores = new int[2];
  cores[0] = 0;
  cores[1] = 1;
  ThreadPool *th = new ThreadPool(cores, 2);
  th->EnqueTask(thread_helper, nullptr);
  Testing testing(th, 0);
  Testing testing1(th, 1);
  Testing testing2(th, 2);
  Testing testing3(th, 3);
  Testing testing4(th, 4);
  sleep(5);
  delete th;
  delete[] cores;
  return 0;
}
