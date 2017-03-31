#include <poll.h>

#include "event_loop.h"
#include "channel.h"

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
// EPOLLIN == POLLIN
// EPOLLPRI == POLLPRI
// EPOLLOUT == POLLOUT
// EPOLLRDHUP == POLLRDHUP
// EPOLLERR == POLLERR
// EPOLLHUP == POLLHUP

const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;
const int Channel::kNoneEvent = 0;

Channel::Channel(EventLoop * loop, int fd):
  loop_(loop),
  fd_(fd),
  events_(0),
  index_(-1)
{
}

void Channel::EnableReadEvent() {
  events_ |= kReadEvent;
  Update();
}

void Channel::DisableReadEvent() {
  events_ &= ~kReadEvent;
  Update();
}

void Channel::EnableWriteEvent() {
  events_ |= kWriteEvent;
  Update();
}

void Channel::DisableWriteEvent() {
  events_ &= ~kWriteEvent;
  Update();
}

void Channel::DisableAllEvent() {
  events_ = kNoneEvent;
  Update();
}

bool Channel::HasWriteEvent() {
  return events_ & kWriteEvent;
}

void Channel::HandleEvent() {
  // POLLHUP Hang up happened on the associated file descriptor.
  if ((events_ & POLLHUP) && !(events_ & POLLIN)) {
    LOG(WARNING) << "Channel handle event EPOLLHUP";
    if (close_callback_) {
        close_callback_();
    }
  }

  // POLLNVAL Invalid request: fd not open
  if (events_ & POLLNVAL) {
    LOG(WARNING) << "Channel handle event POLLNVAL";
  }

  // POLLERR Error condition
  //
  // POLLNVAL Invalid request: fd not open
  if (events_ & (POLLERR | POLLNVAL)) {
    if (error_callback_) {
      error_callback_();
    }
  }

  // POLLIN There is data to read.
  //
  // POLLPRI There is urgent data to read
  //
  // POLLRDHUP Stream  socket  peer  closed  connection,  or  shut down writing half of
  // connection. 
  if (events_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (read_callback_) {
      read_callback_();
    }
  }

  // POLLOUT Writing now will not block.
  if (events_ & POLLOUT) {
    if (write_callback_) {
      write_callback_();
    }
  }
}

void Channel::Remove() {
  loop_->RemoveChannel(this);
}

void Channel::Update() {
  loop_->UpdateChannel(this);
}
