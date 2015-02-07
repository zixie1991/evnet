#include "eventloop.h"

#include "poller.h"
#include "channel.h"

const int kPollTimeOutMs = 10000;

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    poller_(new Poller(this))
{
}

EventLoop::~EventLoop() {
    assert(!looping_);
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        active_channels_.clear();
        poller_->poll(kPollTimeOutMs, active_channels_);

        for (std::vector<Channel*>::iterator it = active_channels_.begin();
                it != active_channels_.end(); ++it) {
            (*it)->handleEvent();
        }

    }

    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}
