#include "workers.h"

#include <boost/bind.hpp>

#include "channel.h"

using boost::bind;

Workers::Workers(EventLoop* loop, WorkerPool* workerpool):
	loop_(loop),
	workerpool_(workerpool),
	channel_(new Channel(loop, workerpool->fd()))
{
	channel_->set_read_callback(bind(&Workers::handleReadEvent, this));
	// we always read the pipe read handler
	channel_->enableReadEvent();
}

Workers::~Workers() {

}

void Workers::delegate(const WorkerPtr& worker) {
	workerpool_->push(worker);
}

void Workers::handleReadEvent() {
	Worker* worker;
	workerpool_->pop(worker);

	worker->callback();
}
