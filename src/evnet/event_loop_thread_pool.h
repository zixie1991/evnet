#ifndef EVNET_EVENT_LOOP_THREAD_POOL_H_
#define EVNET_EVENT_LOOP_THREAD_POOL_H_

#include "common.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
  public:
    typedef function<void(EventLoop*)> Callback;

    EventLoopThreadPool(EventLoop* loop, int thread_num);

    void Start();
    void Stop();

    EventLoop* GetNextLoop();
    EventLoop* GetLoopForHash(size_t hash_code);

    EventLoop* main_loop() {
      return main_loop_;
    }

    int thread_num() {
      return thread_num_;
    }

    void set_name(const string& name) {
      name_ = name;
    }

    const string& name() const {
      return name_;
    }

    bool IsRunning() const {
      return running_;
    }

    bool IsStopped() const {
      return !running_;
    }

    void set_thread_init_callback(const Callback& callback) {
      thread_init_callback_ = callback;
    }

  private:
    EventLoop* main_loop_;
    int thread_num_;
    string name_;
    bool running_;
    vector<shared_ptr<EventLoopThread>> threads_;
    Callback thread_init_callback_;
    int next_;
};

#endif
