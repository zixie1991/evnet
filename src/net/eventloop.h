#ifndef NET_EVENTLOOP_H_
#define NET_EVENTLOOP_H_

#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

#include "util/thread.h"

#include "timerqueue.h"

class Channel;
class Poller;
class Timestamp;
class Wakeup;

class EventLoop {
  public:
    typedef boost::function<void ()> Callback;
    EventLoop();
    ~EventLoop();

    /**
      * @brief Loops foever.
      */
    void Loop();

    void Quit();

    // Run callbacks in the loop
    // Run callback immediately in the loop
    void RunInLoop(const Callback& cb);
    // Push callback to queues callback in the loop
    void QueueInLoop(const Callback& cb);

    // timers
    void RunAt(const Timestamp& when, const TimerQueue::TimerCallback& cb);
    void RunAfter(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb);
    void RunRepeat(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb);

    // internal use only
    void UpdateChannel(Channel* channel);
    void RemoveChannel(Channel* channel);

    // current thread is in eventloop thread.
    bool IsInLoopThread() const {
      return tid_ == gettid();
    }

  private:
    void DoPendingCallbacks();

    bool looping_; // atomic
    bool quit_; // atomic
    boost::scoped_ptr<Poller> poller_;
    boost::scoped_ptr<TimerQueue> timer_queue_;
    // event notification
    boost::scoped_ptr<Wakeup> wakeup_;
    std::vector<Channel*> active_channels_;

    // queue callback
    std::vector<Callback> pending_callbacks_;
    // atomic
    bool calling_pending_callbacks_;

    // ensure eventloop thread safe.
    const pid_t tid_;
    Mutex mutex_;
};

#endif // NET_EVENTLOOP_H_

