#include <sys/time.h>
#include <inttypes.h>

#include "timestamp.h"

Timestamp::Timestamp(long microseconds):
  microseconds_(microseconds)
{
}

Timestamp::~Timestamp() {

}

void Timestamp::Now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long seconds = tv.tv_sec;

  microseconds_ = seconds * kMicrosecondsPerSecond + tv.tv_usec;
}

std::string Timestamp::ToString() const {
  char buf[32] = {0};
  long seconds = microseconds_ / kMicrosecondsPerSecond;
  long microseconds = microseconds_ % kMicrosecondsPerSecond;
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  return buf;
}

std::string Timestamp::ToSimpleString() const {
  char buf[32] = {0};
  long seconds = static_cast<long>(microseconds_ / kMicrosecondsPerSecond);
  int microseconds = static_cast<long>(microseconds_ % kMicrosecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);

  snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d", 
          tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, 
          tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, 
          microseconds);

  return buf;
}

void Timestamp::To(struct timespec* ts) {
  long microseconds = microseconds_;

  // 处理延时
  if (microseconds < 0) {
    ts->tv_sec = 0;
    ts->tv_nsec = 1;
    return ;
  }

  ts->tv_sec = static_cast<long>(microseconds / Timestamp::kMicrosecondsPerSecond);
  ts->tv_nsec = static_cast<long>(microseconds % Timestamp::kMicrosecondsPerSecond * 1000);
}
