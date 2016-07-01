#include "workers.h"

#include <boost/bind.hpp>

#include "channel.h"

using boost::bind;

Workers::Workers(EventLoop* loop, WorkerPool* workerpool):
	loop_(loop),
	workerpool_(workerpool),
	channel_(new Channel(loop, workerpool->fd()))
{
	channel_->set_read_callback(bind(&Workers::HandleReadEvent, this));
	// we always read the pipe read handler
	channel_->EnableReadEvent();
}

Workers::~Workers() {

}

void Workers::Delegate(const WorkerPtr& worker) {
	workerpool_->Push(worker);
}

void Workers::HandleReadEvent() {
	Worker* worker;
	workerpool_->Pop(worker);

	worker->Callback();
}
