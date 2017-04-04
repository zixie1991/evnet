#include <exception>

#include "event_loop.h"
#include "event_loop_thread.h"

EventLoopThread::EventLoopThread(const Callback& callback):
  callback_(callback),
  status_(kStopped)
{}

EventLoopThread::~EventLoopThread() {
  if (thread_ && thread_->joinable()) {
    try {
      thread_->join();
    } catch (const std::system_error& e) {
      LOG(ERROR) << "Caught a system_error:" << e.what();
    }
  }
}

void EventLoopThread::Start() {
  thread_.reset(new std::thread(bind(&EventLoopThread::ThreadFunc, this)));
}

void EventLoopThread::Stop() {
  CHECK(IsRunning());

  status_ = kStopping;
  loop_->Stop();
}

void EventLoopThread::ThreadFunc() {
  status_ = kRunning;
  
  loop_ = shared_ptr<EventLoop>(new EventLoop());

  if (callback_) {
    callback_(loop());
  }

  loop_->Run();

  loop_.reset();
  status_ = kStopped;
}
