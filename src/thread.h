#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>

#include <queue>
#include <string>


class Mutex{
	public:
		Mutex(){
			pthread_mutex_init(&mutex_, NULL);
		}
		~Mutex(){
			pthread_mutex_destroy(&mutex_);
		}
		void lock(){
			pthread_mutex_lock(&mutex_);
		}
		void unlock(){
			pthread_mutex_unlock(&mutex_);
		}
		pthread_mutex_t* pthread_mutex() {
			return &mutex_;
		}

	private:
		pthread_mutex_t mutex_;
};


class Lock{
	public:
		Lock(Mutex *mutex){
			mutex_ = mutex;
			mutex_->lock();
		}
		~Lock(){
			mutex_->unlock();
		}

	private:
		Mutex *mutex_;
		// No copying allowed
		Lock(const Lock&);
		void operator=(const Lock&);
};


class Condition {
	public:
		Condition(Mutex* mutex):
			mutex_(mutex)
		{
			pthread_cond_init(&cond_, NULL);
		}

		~Condition(){
			pthread_cond_destroy(&cond_);
		}

		int wait() {
			return pthread_cond_wait(&cond_, mutex_->pthread_mutex());
		}
		int timedwait(struct timespec& ts) {
			return pthread_cond_timedwait(&cond_, mutex_->pthread_mutex(), &ts);
		}
		int signal() {
			return pthread_cond_broadcast(&cond_);
		}
		int broadcast() {
			return pthread_cond_broadcast(&cond_);
		}

	private:
		pthread_cond_t cond_;
		Mutex *mutex_;
};


// Thread safe queue
template <class T>
class Queue{
	public:
		Queue();
		~Queue();

		bool empty();
		int size();
		int push(const T& item);
		// TODO: with timeout
		int pop(T &data);

	private:
		pthread_cond_t cond_;
		pthread_mutex_t mutex_;
		std::queue<T> items_;
};


// Selectable queue, multi writers, single reader
template <class T>
class SelectableQueue{
	public:
		SelectableQueue();
		~SelectableQueue();
		int fd(){
			return fds_[0];
		}

		// multi writer
		int push(const T& item);
		// single reader
		int pop(T &data);

	private:
		int fds_[2];
		std::queue<T> items_;
};


class Worker {
	public:
		Worker() {

		}
		virtual ~Worker() {

		}

		/**
		* @brief 在执行具体业务前的公共操作
		*/
		virtual int preProcess() {
			return 0;
		};

		/**
		* @brief 具体业务
		*/
		virtual int process() = 0;

		/**
		* @brief 执行具体业务后的公共操作
		*/
		virtual int postProcess() {
			return 0;
		}

		virtual int callback() {
			return 0;
		}

};

typedef Worker* WorkerPtr;

class WorkerPool {
	public:
		WorkerPool(const char *name="");
		~WorkerPool();

		int fd(){
			return results_.fd();
		}

		int start(int num_workers);
		int stop();

		int push(const WorkerPtr& job);
		int pop(WorkerPtr& job);

	private:
		std::string name_;
		Queue<WorkerPtr> jobs_;
		SelectableQueue<WorkerPtr> results_;

		int num_workers_;
		std::vector<pthread_t> tids_;
		bool started_;

		struct run_arg{
			int id;
			WorkerPool *tp;
		};
		static void* run_worker(void *arg);
};


#endif // THREAD_H_
