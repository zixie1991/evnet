#include "eventloop.h"

#include "util/ptime.h"
#include "util/log.h"

#include "poller.h"
#include "channel.h"
#include "timerqueue.h"
#include "wakeup.h"

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

void EventLoop::Loop() {
  looping_ = true;
  quit_ = false;

  while (!quit_) {
    active_channels_.clear();
    poller_->Poll(kPollTimeOutMs, active_channels_);

    for (std::vector<Channel*>::iterator it = active_channels_.begin();
            it != active_channels_.end(); ++it) {
        (*it)->HandleEvent();
    }

    // do pending callbacks
    DoPendingCallbacks();
  }

  log_warn("EventLoop stop looping");
  looping_ = false;
}

void EventLoop::Quit() {
    quit_ = true;
}

void EventLoop::RunInLoop(const Callback& cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueueInLoop(cb);
  }
}

void EventLoop::QueueInLoop(const Callback& cb) {
  {
  Lock lock(&mutex_);
  pending_callbacks_.push_back(cb);
  }

  if (!IsInLoopThread() || calling_pending_callbacks_) {
    wakeup_->Notify();
  }    
}

void EventLoop::DoPendingCallbacks() {
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

void EventLoop::RunAt(const Timestamp& when, const TimerQueue::TimerCallback& cb) {
  timer_queue_->AddTimer(cb, when, 0);
}

void EventLoop::RunAfter(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb) {
  timer_queue_->AddTimer(cb, addtime(when, seconds), 0);
}

void EventLoop::RunRepeat(const Timestamp& when, double seconds, const TimerQueue::TimerCallback& cb) {
  timer_queue_->AddTimer(cb, when, seconds);
}

void EventLoop::UpdateChannel(Channel* channel) {
  poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  poller_->RemoveChannel(channel);
}
