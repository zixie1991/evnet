#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include <vector>

#include <boost/scoped_ptr.hpp>

class Channel;
class Poller;

class EventLoop {
    public:
        EventLoop();
        ~EventLoop();

        /**
         * @brief Loops foever.
         */
        void loop();

        void quit();

        // internal use only
        void updateChannel(Channel* channel);

    private:
        bool looping_;
        bool quit_;
        boost::scoped_ptr<Poller> poller_;
        std::vector<Channel*> active_channels_;
};

#endif // EVENTLOOP_H_
