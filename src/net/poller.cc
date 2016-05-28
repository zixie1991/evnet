#include "poller.h"

#include <sys/epoll.h>

#include "util/log.h"

#include "channel.h"

const int Poller::kInitEventSize;

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

Poller::Poller(EventLoop* loop):
    owner_loop_(loop),
    epollfd_(::epoll_create(EPOLL_CLOEXEC)),
    events_(kInitEventSize)
{
}

Poller::~Poller() {
    ::close(epollfd_);
}

int Poller::poll(int timeout_ms, std::vector<Channel*>& active_channels) {
    int num_events = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeout_ms);
    log_trace("%d channels", channels_.size());

    if (num_events > 0) {
        log_trace("%d events happended", num_events);
        fillActiveChannels(num_events, active_channels);

        if (static_cast<size_t>(num_events) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (num_events == 0) {
        log_trace("nothing happend");
    } else {
        log_trace("Poller poll() error");
    }

    // FIXME meaningless retval
    return 0;
}

void Poller::updateChannel(Channel* channel) {
    log_trace("fd=%d, index=%d, events=%d", channel->fd(), channel->index(), channel->events());
    int index = channel->index();
    
    if (kNew == index || kDeleted == index) {
        // create new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();

        if (kNew == index)  {
            log_trace("fd=%d, index=%d", channel->fd(), channel->index());
            log_trace("before event add: %d channels", channels_.size());
            channels_[fd] = channel;
            log_trace("after event add: %d channels", channels_.size());
        } else { // kDeleted == index
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existed one with EPOLL_CTL_MOD/EPOLL_CTL_DEL
        
        if (0 == channel->events()) {
            // none events
            update(EPOLL_CTL_DEL, channel);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Poller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    int index = channel->index();

    size_t n = channels_.erase(fd);
    (void)n;
    assert(1 == n);

    if (kAdded == index) {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->set_index(kNew);
}

void Poller::fillActiveChannels(int num_events, std::vector<Channel*>& active_channels) {
    for (int i = 0; i < num_events; i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);

        channel->set_events(events_[i].events);
        active_channels.push_back(channel);
    }
}

void Poller::update(int operation, Channel* channel) {
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
