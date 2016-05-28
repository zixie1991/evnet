#ifndef NET_POLLER_H_
#define NET_POLLER_H_

#include <sys/epoll.h>

#include <vector>
#include <map>

#include "eventloop.h"

class Channel;

/**
 * @brief Base class for IO Multiplexing
 */
class Poller {
    public:
        Poller(EventLoop* loop);
        ~Poller();

        /**
         * @brief Polls the I/O events.
         */
        int poll(int timeout_ms, std::vector<Channel*>& channels);

        /**
         * @brief Change the interested I/O events.
         */
        void updateChannel(Channel* channel);

        /**
         * @brief Remove the channel
         */
        void removeChannel(Channel* channel);

    private:
        static const int kInitEventSize = 16;

        void fillActiveChannels(int num_events, std::vector<Channel*>& active_channels);

        void update(int operation, Channel* channel);

        EventLoop* owner_loop_;
        // sorted map
        std::map<int, Channel*> channels_;

        // epoll
        int epollfd_;
        std::vector<struct epoll_event> events_;
};

#endif // NET_POLLER_H_

