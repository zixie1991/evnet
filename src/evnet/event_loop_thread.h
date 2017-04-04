#ifndef EVNET_EVENT_LOOP_THREAD_H_
#define EVNET_EVENT_LOOP_THREAD_H_

#include "common.h"

class EventLoop;

class EventLoopThread {
  public:
    typedef function<void(EventLoop*)> Callback;

    EventLoopThread(const Callback& callback);
    ~EventLoopThread();

    void Start();
    void Stop();

    EventLoop* loop() {
      return loop_.get();
    }

    void set_name(const string& name) {
      name_ = name;
    }

    const string& name() const {
      return name_;
    }

    bool IsRunning() const {
      return status_ == kRunning;
    }

    bool IsStopped() const {
      return status_ == kStopped;
    }

  private:
    void ThreadFunc();

    shared_ptr<EventLoop> loop_;
    Callback callback_;
    shared_ptr<std::thread> thread_;

    string name_;
    enum Status {kRunning, kStopping, kStopped};
    Status status_;
};

#endif
