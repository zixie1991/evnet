#ifndef NET_WAKEUP_H_
#define NET_WAKEUP_H_

#include <boost/scoped_ptr.hpp>

class EventLoop;
class Channel;

class Wakeup {
    public:
        Wakeup(EventLoop* loop);
        ~Wakeup();

        // notify event
        void notify();

    private:
        // called when eventfd notify
        void handleReadEvent();

        EventLoop* loop_;
        // event notification file descriptor, create by eventfd().
        int wakeupfd_;
        boost::scoped_ptr<Channel> wakeupfd_channel_;
};

#endif // NET_WAKEUP_H_

