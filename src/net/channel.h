#ifndef NET_CHANNEL_H_
#define NET_CHANNEL_H_

#include <boost/function.hpp>

#include "eventloop.h"

/**
 * @brief A selectable I/O channel.
 * this class 自始至终只属于一个EventLoop
 * this class 自始至终只负责一个file descriptor的IO事件分发
 * 但它并不拥有这个fd
 * this file descriptor could be a socket,
 * an eventfd, a timerfd, or a signalfd
 */
class Channel {
  public:
    typedef boost::function<void()> EventCallback;

    Channel(EventLoop* loop, int fd);

    void HandleEvent();

    void set_read_callback(const EventCallback& cb) {
      read_callback_ = cb;
    }

    void set_write_callback(const EventCallback& cb) {
        write_callback_ = cb;
    }

    void set_close_callback(const EventCallback& cb) {
        close_callback_ = cb;
    }

    void set_error_callback(const EventCallback& cb) {
        error_callback_ = cb;
    }

    void EnableReadEvent();
    void DisableReadEvent();
    void EnableWriteEvent();
    void DisableWriteEvent();
    void DisableAllEvent();

    bool HasWriteEvent();

    EventLoop* loop() {
      return loop_;
    }

    int fd() const {
      return fd_;
    }

    int events() {
      return events_;
    }

    void set_events(int events) {
      events_ = events;
    }

    // for Poller
    int index() {
      return index_;
    }

    void set_index(int index) {
      index_ = index;
    }

    void Remove();

  private:
    static const int kReadEvent;
    static const int kWriteEvent;
    static const int kNoneEvent;

    void Update();

    EventLoop* loop_;
    int fd_;
    int events_;
    // used by Poller
    int index_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};

#endif // NET_CHANNEL_H_
