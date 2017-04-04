#include "event_loop.h"
#include "event_loop_thread.h"
#include "event_loop_thread_pool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop, int thread_num):
  main_loop_(loop),
  thread_num_(thread_num),
  running_(false),
  next_(0)
{
}

void EventLoopThreadPool::Start() {
  CHECK(!running_);
  running_ = true;

  for (int i = 0; i < thread_num_; i++) {
    shared_ptr<EventLoopThread> t(new EventLoopThread(thread_init_callback_));
    stringstream ss;
    ss << "EventLoopThreadPool-thread-" << i << "th";
    t->set_name(ss.str());
    t->Start();
    threads_.push_back(t);
  }
}

void EventLoopThreadPool::Stop() {
  for (size_t i = 0; i < threads_.size(); i++) {
    threads_[i]->Stop();
  }

  running_ = false;
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  EventLoop* loop = main_loop_;
  if (!threads_.empty()) {
    loop = threads_[next_]->loop();
    next_ = (next_ + 1) % threads_.size();
  }

  return loop;
}

EventLoop* EventLoopThreadPool::GetLoopForHash(size_t hash_code) {
  EventLoop* loop = main_loop_;
  if (!threads_.empty()) {
    loop = threads_[hash_code % threads_.size()]->loop();
  }

  return loop;
}
