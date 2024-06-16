#include "ThreadPool.h"
#include <csignal>
#include <iostream>
#include <unistd.h>

volatile bool run_application;

void *thread_helper(void *arg) {
  std::cout << " Hi from thread helper" << std::endl;
  return nullptr;
}

class Example {
public:
  struct ThreadPoolHelper {
    Example *test;
    int to_run;
    ThreadPoolHelper(Example *test_, int to_run_)
        : test(test_), to_run(to_run_) {}
  };

  Example(ThreadPool *th_, int k) : th(th_) {
    t = new ThreadPoolHelper(this, k);
    AddTaskToQueue();
  };
  Example(Example &&) = default;
  Example(const Example &) = default;
  Example &operator=(Example &&) = default;
  Example &operator=(const Example &) = default;
  ~Example() { delete t; };

  inline void AddTaskToQueue() noexcept { th->EnqueTask(&Example::run, t); }

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

void SignalHelper(int signal) {
  std::cout << " Got kill signal" << std::endl;
  run_application = false;
}

int main(int argc, char *argv[]) {
  run_application = true;
  std::signal(SIGINT, SignalHelper);
  int *cores = new int[2];
  cores[0] = 0;
  cores[1] = 1;
  ThreadPool *th = new ThreadPool(cores, 2);
  th->EnqueTask(thread_helper, nullptr);
  Example testing(th, 0);
  Example testing1(th, 1);
  Example testing2(th, 2);
  Example testing3(th, 3);
  Example testing4(th, 4);
  std::cout << "Infinite loop starting" << std::endl;
  while (run_application) {
  }
  std::cout << "Thread Pool Deleting" << std::endl;
  delete th;
  delete[] cores;
  return 0;
}
