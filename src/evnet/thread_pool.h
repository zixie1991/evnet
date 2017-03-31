#ifndef EVNET_THREAD_POOL_H_
#define EVNET_THREAD_POOL_H_

#include "common.h"

class ThreadPool {
  public:
    typedef function<void()> Task;

    ThreadPool(const string& name=string("ThreadPool"));

    void Start(int thread_num);
    void Stop();

    void Run(const Task& task);
    Task Take();

    void set_name(const string& name) {
      name_ = name;
    }

    const string& name() const {
      return name_;
    }

    void set_thread_init_callback(const Task& callback) {
      thread_init_callback_ = callback;
    }

  private:
    void RunInThread();

    string name_;

    vector<shared_ptr<std::thread>> threads_;
    mutable mutex mutex_;
    condition_variable not_empty_;
    queue<Task> queue_;

    bool running_;
    Task thread_init_callback_;
};

#endif
