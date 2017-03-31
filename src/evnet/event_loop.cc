#include "poller.h"
#include "epoll_poller.h"
#include "channel.h"
#include "event.h"
#include "timer.h"
#include "event_loop.h"

const int kPollTimeOutMs = 10000;

EventLoop::EventLoop():
  running_(false),
  poller_(new EpollPoller(this)),
  calling_pending_callbacks_(false),
  event_(new Event(this)),
  tid_(std::this_thread::get_id())
{
}

EventLoop::~EventLoop() {
  CHECK(!running_);
}

void EventLoop::Run() {
  running_ = true;

  while (running_) {
    active_channels_.clear();
    poller_->Poll(kPollTimeOutMs, active_channels_);

    for (vector<Channel*>::iterator it = active_channels_.begin();
            it != active_channels_.end(); ++it) {
        (*it)->HandleEvent();
    }

    // do pending callbacks
    DoPendingCallbacks();
  }

  LOG(WARNING) << "EventLoop stop looping";
  running_ = false;
}

void EventLoop::Stop() {
  running_ = false;

  // There is a chance that loop() just executes while(running_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!IsInLoopThread()) {
    Wakeup();
  }
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
  lock_guard<mutex> lock(mutex_);
  pending_callbacks_.push_back(cb);
  }

  if (!IsInLoopThread() || calling_pending_callbacks_) {
    Wakeup();
  }
}

void EventLoop::Wakeup() {
  event_->Notify();
}

void EventLoop::DoPendingCallbacks() {
  vector<Callback> callbacks;
  calling_pending_callbacks_ = true;

  {
  lock_guard<mutex> lock(mutex_);
  callbacks.swap(pending_callbacks_);
  }

  for (size_t i = 0; i < callbacks.size(); i++) {
    callbacks[i]();
  }

  calling_pending_callbacks_ = false;
}

shared_ptr<Timer> EventLoop::RunAt(const Callback& cb, const Timestamp& when) {
  shared_ptr<Timer> timer(new Timer(this, cb, when, 0));
  timer->Start();

  return timer;
}

shared_ptr<Timer> EventLoop::RunAfter(const Callback& cb, double delay) {
  Timestamp when;
  when.Now();
  when = when + delay;
  shared_ptr<Timer> timer(new Timer(this, cb, when, 0));
  timer->Start();

  return timer;
}

shared_ptr<Timer> EventLoop::RunRepeat(const Callback& cb, const Timestamp& when, double interval) {
  shared_ptr<Timer> timer(new Timer(this, cb, when, interval));
  timer->Start();

  return timer;
}

void EventLoop::UpdateChannel(Channel* channel) {
  poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  poller_->RemoveChannel(channel);
}
