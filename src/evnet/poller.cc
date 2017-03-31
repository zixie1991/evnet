#include "event_loop.h"
#include "poller.h"

Poller::Poller(EventLoop* loop):
  owner_loop_(loop)
{
}

Poller::~Poller() {
}
