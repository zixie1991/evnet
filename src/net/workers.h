#ifndef NET_WORKERS_H_
#define NET_WORKERS_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "util/thread_pool.h"

class EventLoop;
class Channel;
class Connection;

class Workers {
  public:
    Workers(EventLoop* loop, ThreadPoolWithPipe* thread_pool);
    ~Workers();

    void Delegate(ThreadPoolWithPipe::Task* task);

  private:
    void HandleReadEvent();

    EventLoop* loop_;
    ThreadPoolWithPipe* thread_pool_;
    // thread pool read handler
    int fd_;
    boost::scoped_ptr<Channel> channel_;
};

#endif // NET_WORKERS_H_
