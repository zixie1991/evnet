#include "wakeup.h"

#include <sys/eventfd.h>

#include <boost/bind.hpp>

#include "util/log.h"

#include "channel.h"

using boost::bind;

Wakeup::Wakeup(EventLoop* loop):
  loop_(loop),
  wakeupfd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
  wakeupfd_channel_(new Channel(loop, wakeupfd_))
{
  wakeupfd_channel_->set_read_callback(bind(&Wakeup::HandleReadEvent, this));
  wakeupfd_channel_->EnableReadEvent();
}

Wakeup::~Wakeup() {
  ::close(wakeupfd_);
}

void Wakeup::Notify() {
  uint64_t one = 1;    
  int n = ::write(wakeupfd_, &one, sizeof(one));

  if (sizeof(one) != n) {
    log_error("wakeupfd_ write");
  }
}

void Wakeup::HandleReadEvent() {
  uint64_t one = 1;
  int n = ::read(wakeupfd_, &one, sizeof(one));

  if (sizeof(one) != n) {
    log_error("wakeupfd_ handle read event");
  }
}
