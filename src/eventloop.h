#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include <vector>

#include <boost/scoped_ptr.hpp>

#include "timerqueue.h"

class Channel;
class Poller;
class Timestamp;

class EventLoop {
    public:
        EventLoop();
        ~EventLoop();

        /**
         * @brief Loops foever.
         */
        void loop();

        void quit();

        void runAt(const Timestamp& when, const TimerQueue::TimerCallback& cb);
        void runAfter(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb);
        void runRepeat(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb);

        // internal use only
        void updateChannel(Channel* channel);
        void removeChannel(Channel* channel);

    private:
        bool looping_;
        bool quit_;
        boost::scoped_ptr<Poller> poller_;
        boost::scoped_ptr<TimerQueue> timer_queue_;
        std::vector<Channel*> active_channels_;
};

#endif // EVENTLOOP_H_
