#include "thread_pool.h"

ThreadPool::ThreadPool(const string& name):
  name_(name),
  running_(false)
{}

void ThreadPool::Start(int thread_num) {
  CHECK(threads_.empty());

  running_ = true;
  threads_.reserve(thread_num);
  for (int i = 0; i < thread_num; i++) {
    shared_ptr<std::thread> t(new std::thread(bind(&ThreadPool::RunInThread, this)));
    threads_.push_back(t);
  }
}

void ThreadPool::Stop() {
  unique_lock<mutex> lock(mutex_);
  running_ = false;
  not_empty_.notify_all();
}

void ThreadPool::Run(const Task& task) {
  unique_lock<mutex> lock(mutex_);
  queue_.push(task);
  not_empty_.notify_one();
}

ThreadPool::Task ThreadPool::Take() {
  unique_lock<mutex> lock(mutex_);
  // always use a while-loop, due to spurious wakeup
  while (queue_.empty() && running_) {
    not_empty_.wait(lock);
  }

  Task task;
  if (!queue_.empty()) {
    task = queue_.front();
    queue_.pop();
  }

  return task;
}

void ThreadPool::RunInThread() {
  if (thread_init_callback_) {
    thread_init_callback_();
  }

  while (running_) {
    Task task = queue_.front();
    queue_.pop();
    if (task) {
      task();
    }
  }
}
