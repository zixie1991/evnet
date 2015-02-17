#ifndef TIMERQUEUE_H_
#define TIMERQUEUE_H_

#include <set>
#include <vector>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

class EventLoop;
class Channel;
class Timestamp;
class Timer;

class TimerQueue {
    public:
        typedef boost::function<void()> TimerCallback;

        TimerQueue(EventLoop* loop);
        ~TimerQueue();

        void addTimer(const TimerCallback& cb, Timestamp when, double interval);

    private:
        // called when timerfd alarms
        void handleReadEvent();

        // intert Timer into timers_
        bool insert(Timer* timer);
        std::vector<std::pair<Timestamp, Timer*> > getExpiredTimers(Timestamp when);
        // delete or reset Timer in the timers_
        void deleteOrReset(const std::vector<std::pair<Timestamp, Timer*> >& timers, const Timestamp& now);

        std::set<std::pair<Timestamp, Timer*> > timers_;

        EventLoop* loop_;
        int timerfd_;
        boost::scoped_ptr<Channel> timerfd_channel_;
};

#endif // TIMERQUEUE_H_
