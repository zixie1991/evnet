#ifndef NET_WORKERS_H_
#define NET_WORKERS_H_

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "util/thread.h"

class EventLoop;
class Channel;
class Connection;

class Workers {
  public:
    typedef boost::function<void (const Worker*)> WorkerCallback;

    Workers(EventLoop* loop, WorkerPool* workerpool);
    ~Workers();

    void Delegate(const WorkerPtr& worker);

    void set_callback(const WorkerCallback& cb) {
        callback_ = cb;
    }

  private:
    void HandleReadEvent();

    EventLoop* loop_;
    WorkerPool* workerpool_;
    // worker pool read handler
    int fd_;
    boost::scoped_ptr<Channel> channel_;

    WorkerCallback callback_;
};

#endif // NET_WORKERS_H_
