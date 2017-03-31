#include <sys/eventfd.h>

#include "event_loop.h"
#include "channel.h"
#include "event.h"

Event::Event(EventLoop* loop):
  eventfd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
  channel_(new Channel(loop, eventfd_))
{
  channel_->set_read_callback(bind(&Event::HandleReadEvent, this));
  channel_->EnableReadEvent();
}

Event::~Event() {
  ::close(eventfd_);
}

void Event::Notify() {
  uint64_t one = 1;    
  int n = ::write(eventfd_, &one, sizeof(one));

  if (sizeof(one) != n) {
    LOG(ERROR) << "eventfd_ wirte " << n << " bytes instead of " << sizeof(one);
  }
}

void Event::HandleReadEvent() {
  uint64_t one = 1;
  int n = ::read(eventfd_, &one, sizeof(one));

  if (sizeof(one) != n) {
    LOG(ERROR) << "eventfd_ read " << n << " bytes instead of " << sizeof(one);
  }
}
