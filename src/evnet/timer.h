#ifndef EVNET_TIMER_H_
#define EVNET_TIMER_H_

#include "common.h"
#include "timestamp.h"

class EventLoop;
class Channel;

class Timer {
  public:
    typedef function<void()> TimerCallback;

    Timer(EventLoop* loop, const TimerCallback& cb, const Timestamp& when, double interval);
    ~Timer();

    void Start();
    void Cancel();

  private:
    // called when timerfd alarms
    void HandleReadEvent();

    TimerCallback callback_;

    int timerfd_;
    unique_ptr<Channel> channel_; 

    Timestamp expiration_;
    double interval_;
    bool repeat_;
};

#endif
