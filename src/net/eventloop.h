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
        void loop();

        void quit();

        // Run callbacks in the loop
        // Run callback immediately in the loop
        void runInLoop(const Callback& cb);
        // Push callback to queues callback in the loop
        void queueInLoop(const Callback& cb);

        // timers
        void runAt(const Timestamp& when, const TimerQueue::TimerCallback& cb);
        void runAfter(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb);
        void runRepeat(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb);

        // internal use only
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);

        // current thread is in eventloop thread.
        bool isInLoopThread() const {
            return tid_ == gettid();
        }

    private:
        void doPendingCallbacks();

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

