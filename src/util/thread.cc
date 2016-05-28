#include "thread.h"

#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>

#include <string.h>
#include <assert.h>

#include "log.h"

template <class T>
Queue<T>::Queue(){
	pthread_cond_init(&cond_, NULL);
	pthread_mutex_init(&mutex_, NULL);
}

template <class T>
Queue<T>::~Queue(){
	pthread_cond_destroy(&cond_);
	pthread_mutex_destroy(&mutex_);
}

template <class T>
bool Queue<T>::empty(){
	bool ret = false;
	if(pthread_mutex_lock(&mutex_) != 0){
		return -1;
	}
	ret = items_.empty();
	pthread_mutex_unlock(&mutex_);
	return ret;
}

template <class T>
int Queue<T>::size(){
	int ret = -1;
	if(pthread_mutex_lock(&mutex_) != 0){
		return -1;
	}
	ret = items_.size();
	pthread_mutex_unlock(&mutex_);
	return ret;
}

template <class T>
int Queue<T>::push(const T& item){
	if(pthread_mutex_lock(&mutex_) != 0){
		return -1;
	}
	{
		items_.push(item);
	}
	pthread_mutex_unlock(&mutex_);
	pthread_cond_signal(&cond_);
	return 1;
}

template <class T>
int Queue<T>::pop(T &data){
	if(pthread_mutex_lock(&mutex_) != 0){
		return -1;
	}
	{
		// 必须放在循环中, 因为 pthread_cond_wait 可能抢不到锁而被其它处理了
		while(items_.empty()){
			//fprintf(stderr, "%d wait\n", pthread_self());
			if(pthread_cond_wait(&cond_, &mutex_) != 0){
				//fprintf(stderr, "%s %d -1!\n", __FILE__, __LINE__);
				return -1;
			}
			//fprintf(stderr, "%d wait 2\n", pthread_self());
		}
		data = items_.front();
		//fprintf(stderr, "%d job: %d\n", pthread_self(), (int)*data);
		items_.pop();
	}
	if(pthread_mutex_unlock(&mutex_) != 0){
		//fprintf(stderr, "error!\n");
		return -1;
	}
    //fprintf(stderr, "%d wait end 2, job: %d\n", pthread_self(), (int)*data);
	return 1;
}


template <class T>
SelectableQueue<T>::SelectableQueue(){
	if(pipe(fds_) == -1){
		exit(0);
	}
}

template <class T>
SelectableQueue<T>::~SelectableQueue(){
	close(fds_[0]);
	close(fds_[1]);
}

template <class T>
int SelectableQueue<T>::push(const T& item){
	if (::write(fds_[1], (char *)&item, sizeof(T)) == -1) {
		exit(0);
	}

	return 1;
}

template <class T>
int SelectableQueue<T>::pop(T &data){
	int n;
	int ret = 0;
	n = ::read(fds_[0], (char *)&data, sizeof(T));
	if (n < 0) {
		if (errno != EINTR) {
			return -1;
		}
	} else if (n == 0) {
		ret = -1;
	}

	return ret;
}


WorkerPool::WorkerPool(const char *name){
	name_ = name;
	started_ = false;
}

WorkerPool::~WorkerPool(){
	if(started_){
		stop();
	}
}

int WorkerPool::push(const WorkerPtr& job){
	return jobs_.push(job);
}

int WorkerPool::pop(WorkerPtr& result){
	return results_.pop(result);
}

void* WorkerPool::run_worker(void *arg){
	struct run_arg *p = (struct run_arg*)arg;
	int id = p->id;
    (void)id;
	WorkerPool *tp = p->tp;
	delete p;

	while(1){
		WorkerPtr job;
		if(tp->jobs_.pop(job) == -1){
			fprintf(stderr, "jobs.pop error\n");
			::exit(0);
			break;
		}

		job->preProcess();
		job->process();
		job->postProcess();

		if(tp->results_.push(job) == -1){
			fprintf(stderr, "results.push error\n");
			::exit(0);
			break;
		}
	}
	return (void *)NULL;
}

int WorkerPool::start(int num_workers){
	num_workers_ = num_workers;
	if(started_){
		return 0;
	}
	int err;
	pthread_t tid;
	for(int i=0; i<num_workers; i++){
		struct run_arg *arg = new run_arg();
		arg->id = i;
		arg->tp = this;

		err = pthread_create(&tid, NULL, &WorkerPool::run_worker, arg);
		if(err != 0){
			fprintf(stderr, "can't create thread: %s\n", strerror(err));
		}else{
			tids_.push_back(tid);
		}
	}
	started_ = true;
	return 0;
}

int WorkerPool::stop(){
	// TODO: notify works quit and wait
	for(size_t i = 0; i < tids_.size(); i++){
		pthread_cancel(tids_[i]);
	}
	return 0;
}


// gettid() returns the caller’s thread ID (TID). In a single-threaded 
// process, the thread ID is equal to the process ID
// In a multithreaded process, all threads have the same PID, but each one 
// has a unique TID
pid_t gettid() {
    //return static_cast<pid_t>(syscall(__NR_gettid));
    return static_cast<pid_t>(syscall(SYS_gettid));
}


Thread::Thread(const std::string& name):
    name_(name),
    pthreadid_(0),
    tid_(0),
    started_(false),
    joined_(false)
{
    context_ = 0;
}

Thread::~Thread() {
    if (started_ && !joined_) {
        pthread_detach(pthreadid_);
    }
}

void Thread::start(void *context) {
    assert(!started_);
    started_ = true;
    context_ = context;
    int ret = pthread_create(&pthreadid_, NULL, threadFunc, this);

    if (0 != ret) {
        started_ = false;
        log_fatal("failed in pthread_create");
        abort();
    }
}

int Thread::join() {
    assert(started_);
    assert(!joined_);
    started_ = false;
    joined_ = true;

    int ret = pthread_join(pthreadid_, 0);
    if (0 != ret) {
        return -1;
    }

    return 0;
}

void* Thread::threadFunc(void *obj) {
    Thread *thread= (Thread *)obj;
    // 注意gettid()的调用位置
    thread->tid_ = gettid(); 
    thread->run();

    return 0;
}
