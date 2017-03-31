#ifndef EVNET_EPOLL_POLLER_H_
#define EVNET_EPOLL_POLLER_H_

#include <sys/epoll.h>

#include "common.h"
#include "poller.h"

class EventLoop;

class EpollPoller: public Poller {
  public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller();

    /**
      * @brief Polls the I/O events.
      */
    int Poll(int timeout_ms, ChannelList& channels);

    /**
      * @brief Change the interested I/O events.
      */
    void UpdateChannel(Channel* channel);

    /**
      * @brief Remove the channel
      */
    void RemoveChannel(Channel* channel);

  private:
    static const int kInitEventSize = 16;

    void FillActiveChannels(int num_events, ChannelList& active_channels);

    void Update(int operation, Channel* channel);

    // epoll
    int epollfd_;
    vector<struct epoll_event> events_;
};

#endif
