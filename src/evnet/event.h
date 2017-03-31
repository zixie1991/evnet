#ifndef EVENT_EVENT_H_
#define EVENT_EVENT_H_

#include "common.h"

class Channel;

/**
 * @brief 目的是唤醒EventLoop owner线程，及时执行pending_callbacks_中任务
 */
class Event {
    public:
      Event(EventLoop* loop);
      ~Event();

      // notify event
      void Notify();

    private:
      // called when eventfd notify
      void HandleReadEvent();

      // event notification file descriptor, create by eventfd().
      int eventfd_;
      unique_ptr<Channel> channel_;
};

#endif
