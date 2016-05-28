#ifndef NET_TIMER_H_
#define NET_TIMER_H_

#include <boost/function.hpp>

#include "util/ptime.h"

class Timer {
    public:
        typedef boost::function<void ()> TimerCallback;

        Timer(const TimerCallback& cb, Timestamp when, double interval);
        ~Timer();

        void run() const {
            callback_();
        }

        void set_expiration(Timestamp expiration) {
            expiration_ = expiration;
        }

        Timestamp expiration() {
            return expiration_;
        }

        double interval() {
            return interval_;
        }

        bool repeat() {
            return repeat_;
        }

    private:
        TimerCallback callback_;

        Timestamp expiration_;
        double interval_;
        bool repeat_;

};

#endif // NET_TIMER_H_

