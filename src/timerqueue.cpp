#include "timerqueue.h"

#include <sys/timerfd.h>

#include <errno.h>

#include <boost/bind.hpp>

#include "channel.h"
#include "ptime.h"
#include "timer.h"
#include "log.h"

using boost::bind;
using std::set;
using std::pair;
using std::vector;

struct timespec timestamp_to_timespec(const Timestamp& when) {
    long microseconds = when.microseconds();
    struct timespec ts;

    // 处理延时
    if (microseconds < 0) {
        ts.tv_sec = 0;
        ts.tv_nsec = 1;
        return ts;
    }

    ts.tv_sec = static_cast<long>(microseconds / Timestamp::kMicrosecondsPerSecond);
    ts.tv_nsec = static_cast<long>(microseconds % Timestamp::kMicrosecondsPerSecond * 1000);

    return ts;
}

TimerQueue::TimerQueue(EventLoop* loop):
    loop_(loop),
    timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
    timerfd_channel_(new Channel(loop, timerfd_))
{
    timerfd_channel_->set_read_callback(bind(&TimerQueue::handleReadEvent, this));
    // we always read the timerfd, and we disarm it with timerfd_settime.
    timerfd_channel_->enableReadEvent();
}

TimerQueue::~TimerQueue() {
    ::close(timerfd_);

    for (set<pair<Timestamp, Timer*> >::iterator iter = timers_.begin(); \
            iter != timers_.end(); ++iter) {
        delete iter->second;
    }
}

void TimerQueue::addTimer(const TimerCallback& cb, Timestamp when, double interval) {
    Timer* timer = new Timer(cb, when, interval);

    bool changed = insert(timer);

    if (changed) {
        struct itimerspec new_value;
        struct itimerspec old_value;

        bzero(&new_value, sizeof(new_value));
        bzero(&old_value, sizeof(old_value));

        // use relative timer
        Timestamp now;
        now.now();
        new_value.it_value = timestamp_to_timespec(Timestamp(when.microseconds() - now.microseconds()));

        if (-1 == ::timerfd_settime(timerfd_, 0, &new_value, &old_value)) {
            log_error("starts timer error: %s", strerror(errno));
            return ;
        }
    }

}

void TimerQueue::handleReadEvent() {
    Timestamp now;
    now.now();

    vector<pair<Timestamp, Timer*> > expired_timers = getExpiredTimers(now);

    for (vector<pair<Timestamp, Timer*> >::iterator iter = expired_timers.begin();
            iter != expired_timers.end(); ++iter) {
        iter->second->run();
    }

    deleteOrReset(expired_timers, now);
}

bool TimerQueue::insert(Timer* timer) {
    Timestamp when = timer->expiration();
    set<pair<Timestamp, Timer*> >::iterator iter = timers_.begin();
    bool retval = false;

    if (iter == timers_.end() || when < iter->first) {
        retval = true;
    }

    timers_.insert(pair<Timestamp, Timer*>(when, timer));
    
    return retval;
}

std::vector<std::pair<Timestamp, Timer*> > TimerQueue::getExpiredTimers(Timestamp when) {
    pair<Timestamp, Timer*> bound(when, NULL);
    set<pair<Timestamp, Timer*> >::iterator end_iter = timers_.lower_bound(bound);
    vector<pair<Timestamp, Timer*> > expired_timers;
    copy(timers_.begin(), end_iter, back_inserter(expired_timers));
    timers_.erase(timers_.begin(), end_iter);

    return expired_timers;
}

void TimerQueue::deleteOrReset(const std::vector<std::pair<Timestamp, Timer*> >& timers, const Timestamp& now) {
    bool changed = false;
    Timestamp min_timestamp;
    for (vector<pair<Timestamp, Timer*> >::const_iterator iter = timers.begin(); \
            iter != timers.end(); ++iter) {
        if (iter->second->repeat()) {
            iter->second->set_expiration(addtime(now, iter->second->interval()));
            if (insert(iter->second)) {
                changed = true;
                min_timestamp = iter->second->expiration();
            }
        } else {
            delete iter->second;
        }
    }

    if (changed) {
        struct itimerspec new_value;
        struct itimerspec old_value;

        bzero(&new_value, sizeof(new_value));
        bzero(&old_value, sizeof(old_value));

        // use relative timer
        Timestamp now;
        now.now();
        new_value.it_value = timestamp_to_timespec(Timestamp(min_timestamp.microseconds() - now.microseconds()));

        if (-1 == ::timerfd_settime(timerfd_, 0, &new_value, &old_value)) {
            log_error("starts timer error: %s", strerror(errno));
            return ;
        }
    }
}
