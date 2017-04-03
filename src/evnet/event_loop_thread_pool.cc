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
  EventLoop* loop = NULL;
  if (!threads_.empty()) {
    loop = threads_[next_]->event_loop();
    next_ += 1;
    next_ = next_ % threads_.size();
  }

  return loop;
}

EventLoop* EventLoopThreadPool::GetLoopForHash(size_t hash_code) {
  EventLoop* loop = NULL;
  if (!threads_.empty()) {
    loop = threads_[hash_code % threads_.size()]->event_loop();
  }

  return loop;
}
