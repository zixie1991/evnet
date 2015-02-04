#include "channel.h"

#include <poll.h>

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
// EPOLLIN == POLLIN
// EPOLLPRI == POLLPRI
// EPOLLOUT == POLLOUT
// EPOLLRDHUP == POLLRDHUP
// EPOLLERR == POLLERR
// EPOLLHUP == POLLHUP

Channel::Channel(EventLoop * loop, int fd):
    loop_(loop),
    fd_(fd),
    events_(0),
    index_(-1)
{
}

void Channel::handleEvent() {
    // POLLNVAL Invalid request: fd not open
    if (events_ & POLLNVAL) {
        //
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
