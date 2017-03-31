#ifndef EVNET_POLLER_H_
#define EVNET_POLLER_H_

#include "common.h"

class EventLoop;
class Channel;

/**
 * @brief Base class for IO Multiplexing
 */
class Poller {
  public:
    typedef vector<Channel*> ChannelList;
    // sorted map
    typedef map<int, Channel*> ChannelMap;

    Poller(EventLoop* loop);
    virtual ~Poller();

    /**
      * @brief Polls the I/O events.
      */
    virtual int Poll(int timeout_ms, ChannelList& channels) {
      LOG(INFO) << "test";
      (void)timeout_ms;
      (void)channels;
      return 0;
    }

    /**
      * @brief Change the interested I/O events.
      */
    virtual void UpdateChannel(Channel* channel) {
      (void)channel;
    }

    /**
      * @brief Remove the channel
      */
    virtual void RemoveChannel(Channel* channel) {
      (void)channel;
    }

  protected:
    ChannelMap channels_;
    EventLoop* owner_loop_;
};

#endif

