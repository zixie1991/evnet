#ifndef UTIL_THREAD_H_
#define UTIL_THREAD_H_

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

		void Lock(){
			pthread_mutex_lock(&mutex_);
		}

		void Unlock(){
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
			mutex_->Lock();
		}

		~Lock(){
			mutex_->Unlock();
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

		int Wait() {
			return pthread_cond_wait(&cond_, mutex_->pthread_mutex());
		}

		int TimedWait(struct timespec& ts) {
			return pthread_cond_timedwait(&cond_, mutex_->pthread_mutex(), &ts);
		}

		int Signal() {
			return pthread_cond_broadcast(&cond_);
		}

		int Broadcast() {
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
		int Push(const T& item);
		// TODO: with timeout
		int Pop(T &data);

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
		int Push(const T& item);
		// single reader
		int Pop(T &data);

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
		virtual int PreProcess() {
			return 0;
		};

		/**
		* @brief 具体业务
		*/
		virtual int Process() = 0;

		/**
		* @brief 执行具体业务后的公共操作
		*/
		virtual int PostProcess() {
			return 0;
		}

		virtual int Callback() {
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

		int Start(int num_workers);
		int Stop();

		int Push(const WorkerPtr& job);
		int Pop(WorkerPtr& job);

	private:
		std::string name_;
		Queue<WorkerPtr> jobs_;
		SelectableQueue<WorkerPtr> results_;

		int num_workers_;
		std::vector<pthread_t> tids_;
		bool started_;

		struct RunArg {
			int id;
			WorkerPool *tp;
		};
		static void* run_worker(void *arg);
};

pid_t gettid();

class Thread {
  public:
    Thread(const std::string& name=std::string());
    virtual ~Thread();

    // abort if thread start error
    void Start(void *context);
    // return pthread_join()
    int Join();

    std::string name() const {
      return name_;
		}

    pthread_t pthreadid() const {
			return pthreadid_;
		}

    pid_t tid() const {
			return tid_;
		}

    bool started() const {
			return started_;
		}

  protected:
    virtual int Run() = 0;

  protected:
    void *context_;

  private:
    static void* thread_func(void *obj);
    std::string name_;
    pthread_t pthreadid_;
    pid_t tid_;
    bool started_;
    bool joined_;
};

#endif // UTIL_THREAD_H_
