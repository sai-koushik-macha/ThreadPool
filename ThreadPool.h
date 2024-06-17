#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <deque>
#include <pthread.h>
#include <sched.h>

class ThreadPool {
public:
  ThreadPool(int *cores_, int total_threads_)
      : cores(cores_), total_threads(total_threads_), core_iter(0),
        keep_running(true) {

    start_running_tasks = false;

    pthread_spin_init(&sp, PTHREAD_PROCESS_PRIVATE);

    threads = new pthread_t[total_threads];

    for (int i = 0; i < total_threads; i++) {
      pthread_create(&threads[i], nullptr, thread_run_helper, this);
    }

    while (true) {
      pthread_spin_lock(&sp);
      int core_iter_ = core_iter;
      pthread_spin_unlock(&sp);
      if (core_iter_ == total_threads) {
        break;
      }
    }

    start_running_tasks = true;
  }
  ~ThreadPool() {

    keep_running = false;

    for (int i = 0; i < total_threads; i++) {
      pthread_join(threads[i], nullptr);
    }

    delete[] threads;

    pthread_spin_destroy(&sp);
  }

  inline void EnqueTask(void *(*const taskFuncPtr)(void *),
                        void *const arg) noexcept {
    pthread_spin_lock(&sp);
    tasks_queue.emplace_back(taskFuncPtr, arg);
    pthread_spin_unlock(&sp);
  }

  inline bool EnqueTaskBasedOnAvailability(void *(*const taskFuncPtr)(void *),
                                           void *const arg) noexcept {
    pthread_spin_lock(&sp);
    if (tasks_queue.size() >= total_threads) {
      pthread_spin_unlock(&sp);
      return false;
    }
    tasks_queue.emplace_back(taskFuncPtr, arg);
    pthread_spin_unlock(&sp);
    return true;
  }

private:
  struct Task {
    void *(*const taskFuncPtr)(void *);
    void *const arg;

    Task(void *(*const taskFuncPtr_)(void *), void *const arg_)
        : taskFuncPtr(taskFuncPtr_), arg(arg_) {}
  };

  volatile bool keep_running;
  volatile bool start_running_tasks;
  const int total_threads;
  int core_iter;
  pthread_spinlock_t sp;
  int *cores;
  pthread_t *threads;
  std::deque<Task> tasks_queue;

  inline static void *thread_run_helper(void *arg) noexcept {
    ThreadPool *obj = static_cast<ThreadPool *>(arg);
    obj->run();
    return nullptr;
  }

  inline void AssignCore(int core) noexcept {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
  }

  inline void run() noexcept {
    pthread_spin_lock(&sp);
    int core = cores[core_iter];
    core_iter++;
    pthread_spin_unlock(&sp);

    AssignCore(core_iter);

    while (!start_running_tasks) {
    }

    while (keep_running) {
      void *(*taskFuncPtr)(void *);
      void *arg;
      bool task_avaiable = false;
      pthread_spin_lock(&sp);
      if (tasks_queue.size() > 0) {
        taskFuncPtr = tasks_queue.front().taskFuncPtr;
        arg = tasks_queue.front().arg;
        tasks_queue.pop_front();
        task_avaiable = true;
      }
      pthread_spin_unlock(&sp);
      if (task_avaiable) {
        taskFuncPtr(arg);
      }
    }
  }
};

#endif /* THREADPOOL_H_ */
