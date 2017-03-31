#include <exception>

#include "event_loop.h"
#include "event_loop_thread.h"

EventLoopThread::EventLoopThread(const Callback& callback):
  event_loop_(new EventLoop()),
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
  event_loop_->Stop();
}

void EventLoopThread::ThreadFunc() {
  status_ = kRunning;
  callback_(event_loop());

  event_loop_->Run();

  event_loop_.reset();
  status_ = kStopped;
}
