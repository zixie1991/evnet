#ifndef UTIL_LOG_H_
#define UTIL_LOG_H_

#include <sys/stat.h>
#include <sys/time.h>
#include <limits.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <string>
#include <sstream>

using std::string;
using std::stringstream;


class Logger{
    public:
        static const int LEVEL_NONE     = (-1);
        static const int LEVEL_MIN      = 0;
        static const int LEVEL_FATAL    = 0;
        static const int LEVEL_ERROR    = 1;
        static const int LEVEL_WARN     = 2;
        static const int LEVEL_INFO     = 3;
        static const int LEVEL_DEBUG    = 4;
        static const int LEVEL_TRACE    = 5;
        static const int LEVEL_MAX      = 5;

        static int get_level(const char *levelname);
    private:
        FILE *fp;
        char filename[PATH_MAX];
        int level_;
        pthread_mutex_t *mutex;

        uint64_t rotate_size;
        struct{
            uint64_t w_curr;
            uint64_t w_total;
        }stats;

        void rotate();
        void threadsafe();
    public:
        Logger();
        ~Logger();

        int level(){
            return level_;
        }

        void set_level(int level){
            this->level_ = level;
        }

        int open(FILE *fp, int level=LEVEL_DEBUG, bool is_threadsafe=false);
        int open(const char *filename, int level=LEVEL_DEBUG,
            bool is_threadsafe=false, uint64_t rotate_size=0);
        void close();

        int logv(int level, const char *fmt, va_list ap);

        int trace(const char *fmt, ...);
        int debug(const char *fmt, ...);
        int info(const char *fmt, ...);
        int warn(const char *fmt, ...);
        int error(const char *fmt, ...);
        int fatal(const char *fmt, ...);
};

extern Logger logger;

int log_open(FILE *fp, int level=Logger::LEVEL_DEBUG, bool is_threadsafe=false);
int log_open(const char *filename, int level=Logger::LEVEL_DEBUG,
    bool is_threadsafe=false, uint64_t rotate_size=0);
int log_level();
void set_log_level(int level);

// __PRETTY_FUNCTION__ replace __FUNCTION__
#define log_trace(fmt, args...) logger.trace("%s:%d] "fmt, __FILE__, __LINE__, ##args)
#define log_debug(fmt, args...) logger.debug("%s:%d] "fmt, __FILE__, __LINE__, ##args)
#define log_info(fmt, args...) logger.info("%s:%d] "fmt, __FILE__, __LINE__, ##args)
#define log_warn(fmt, args...) logger.warn("%s:%d] "fmt, __FILE__, __LINE__, ##args)
#define log_error(fmt, args...) logger.error("%s:%d] "fmt, __FILE__, __LINE__, ##args)
#define log_fatal(fmt, args...) logger.fatal("%s:%d] "fmt, __FILE__, __LINE__, ##args)

#endif // UTIL_LOG_H_

