#include <sys/epoll.h>

#include "channel.h"
#include "event_loop.h"
#include "epoll_poller.h"

const int EpollPoller::kInitEventSize;

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop):
  Poller(loop),
  epollfd_(::epoll_create(EPOLL_CLOEXEC)),
  events_(kInitEventSize)
{
}

EpollPoller::~EpollPoller() {
  ::close(epollfd_);
}

int EpollPoller::Poll(int timeout_ms, ChannelList& active_channels) {
  int num_events = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeout_ms);

  if (num_events > 0) {
    FillActiveChannels(num_events, active_channels);

    if (static_cast<size_t>(num_events) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (num_events == 0) {
  } else {
  }

  // FIXME meaningless retval
  return 0;
}

void EpollPoller::UpdateChannel(Channel* channel) {
  int index = channel->index();
  
  if (kNew == index || kDeleted == index) {
    // create new one, add with EPOLL_CTL_ADD
    int fd = channel->fd();

    if (kNew == index)  {
      channels_[fd] = channel;
    } else { // kDeleted == index
      CHECK(channels_.find(fd) != channels_.end());
      CHECK(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    Update(EPOLL_CTL_ADD, channel);
  } else {
    // update existed one with EPOLL_CTL_MOD/EPOLL_CTL_DEL
    
    if (0 == channel->events()) {
      // none events
      Update(EPOLL_CTL_DEL, channel);
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EpollPoller::RemoveChannel(Channel* channel) {
  int fd = channel->fd();
  int index = channel->index();

  size_t n = channels_.erase(fd);
  (void)n;
  CHECK(1 == n);

  if (kAdded == index) {
    Update(EPOLL_CTL_DEL, channel);
  }

  channel->set_index(kNew);
}

void EpollPoller::FillActiveChannels(int num_events, ChannelList& active_channels) {
  for (int i = 0; i < num_events; i++) {
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);

    channel->set_events(events_[i].events);
    active_channels.push_back(channel);
  }
}

void EpollPoller::Update(int operation, Channel* channel) {
  struct epoll_event event;
  bzero(&event, sizeof(event));
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();

  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (EPOLL_CTL_DEL == operation) {
      // FIXME error
    } else {
      // FIXME error
    }
  }
}
