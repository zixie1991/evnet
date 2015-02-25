#include "eventloop.h"

#include "poller.h"
#include "channel.h"
#include "timerqueue.h"
#include "wakeup.h"
#include "ptime.h"
#include "log.h"

using std::vector;

const int kPollTimeOutMs = 10000;

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    poller_(new Poller(this)),
    timer_queue_(new TimerQueue(this)),
    wakeup_(new Wakeup(this)),
    calling_pending_callbacks_(false),
    tid_(gettid())
{
}

EventLoop::~EventLoop() {
    assert(!looping_);
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        active_channels_.clear();
        poller_->poll(kPollTimeOutMs, active_channels_);

        for (std::vector<Channel*>::iterator it = active_channels_.begin();
                it != active_channels_.end(); ++it) {
            (*it)->handleEvent();
        }

        // do pending callbacks
        doPendingCallbacks();
    }

    log_warn("EventLoop stop looping");
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
}

void EventLoop::runInLoop(const Callback& cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Callback& cb) {
    {
    Lock lock(&mutex_);
    pending_callbacks_.push_back(cb);
    }

    if (!isInLoopThread() || calling_pending_callbacks_) {
        wakeup_->notify();
    }    
}

void EventLoop::doPendingCallbacks() {
    vector<Callback> callbacks;
    calling_pending_callbacks_ = true;

    {
    Lock lock(&mutex_);
    callbacks.swap(pending_callbacks_);
    }

    for (size_t i = 0; i < callbacks.size(); i++) {
        callbacks[i]();
    }

    calling_pending_callbacks_ = false;
}

void EventLoop::runAt(const Timestamp& when, const TimerQueue::TimerCallback& cb) {
    timer_queue_->addTimer(cb, when, 0);
}

void EventLoop::runAfter(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb) {
    timer_queue_->addTimer(cb, addtime(when, seconds), 0);
}

void EventLoop::runRepeat(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb) {
    timer_queue_->addTimer(cb, when, seconds);
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}
