#ifndef EVNET_EVENT_LOOP_H_
#define EVNET_EVENT_LOOP_H_

#include "common.h"
#include "timestamp.h"

class Channel;
class Poller;
class Event;
class Timer;

class EventLoop {
  public:
    typedef function<void()> Callback;

    EventLoop();
    ~EventLoop();

    void Run();
    void Stop();

    // Run callbacks in the loop
    // Run callback immediately in the loop
    void RunInLoop(const Callback& cb);
    // Push callback to queues callback in the loop
    void QueueInLoop(const Callback& cb);
    void Wakeup();

    shared_ptr<Timer> RunAt(const Callback& cb, const Timestamp& when);
    shared_ptr<Timer> RunAfter(const Callback& cb, double delay); // seconds
    shared_ptr<Timer> RunRepeat(const Callback& cb, const Timestamp& when, double interval); // seconds

    // internal use only
    void UpdateChannel(Channel* channel);
    void RemoveChannel(Channel* channel);

    // current thread is in eventloop thread.
    bool IsInLoopThread() const {
      return tid_ == std::this_thread::get_id();
    }

    bool running() const {
      return running_;
    }

    bool IsRunning() const {
      return running();
    }

    bool IsStoped() const {
      return !running();
    }

    const thread::id& tid() const {
      return tid_;
    }

  private:
    EventLoop(const EventLoop&);
    void operator=(const EventLoop&);

    void DoPendingCallbacks();
    void StopInLoop();

    bool running_;

    unique_ptr<Poller> poller_;
    vector<Channel*> active_channels_;

    // 待解决的回调函数
    bool calling_pending_callbacks_;
    vector<Callback> pending_callbacks_;
    unique_ptr<Event> event_;

    // 保证线程安全
    thread::id tid_;
    mutex mutex_;
};

#endif
