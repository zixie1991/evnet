#include <sys/timerfd.h>

#include "event_loop.h"
#include "channel.h"
#include "timer.h"

Timer::Timer(EventLoop* loop, const TimerCallback& cb, const Timestamp& when, double interval):
  callback_(cb),
  timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
  channel_(new Channel(loop, timerfd_)),
  expiration_(when),
  interval_(interval)
{
  channel_->set_read_callback(bind(&Timer::HandleReadEvent, this));
  // we always read the timerfd, and we disarm it with timerfd_settime.
  channel_->EnableReadEvent();
}

Timer::~Timer() {
  ::close(timerfd_);
}

void Timer::Start() {
  struct itimerspec new_value;
  struct itimerspec old_value;

  bzero(&new_value, sizeof(new_value));
  bzero(&old_value, sizeof(old_value));

  // use relative timer
  Timestamp now;
  now.Now();
  Timestamp relative_when = Timestamp(expiration_.microseconds() - now.microseconds());
  relative_when.To(&new_value.it_value);
  Timestamp interval_ts(interval_);
  interval_ts.To(&new_value.it_interval);
  if (-1 == ::timerfd_settime(timerfd_, 0, &new_value, &old_value)) {
    LOG(ERROR) << "Starts timer error: " << strerror(errno);
    return ;
  }
}

void Timer::Cancel() {
  ::close(timerfd_);
}

void Timer::HandleReadEvent() {
  callback_();
}
