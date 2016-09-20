#include "workers.h"

#include <boost/bind.hpp>

#include "channel.h"

using boost::bind;

Workers::Workers(EventLoop* loop, ThreadPoolWithPipe* thread_pool):
	loop_(loop),
	thread_pool_(thread_pool),
	channel_(new Channel(loop, thread_pool_->fd()))
{
	channel_->set_read_callback(bind(&Workers::HandleReadEvent, this));
	// we always read the pipe read handler
	channel_->EnableReadEvent();
}

Workers::~Workers() {

}

void Workers::Delegate(ThreadPoolWithPipe::Task* task) {
	thread_pool_->Run(task);
}

void Workers::HandleReadEvent() {
  ThreadPoolWithPipe::Task* task = thread_pool_->Take();

  task->Callback();
}
