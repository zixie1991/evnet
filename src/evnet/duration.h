// Modeled after the time.Duration of Golang project.
#ifndef EVNET_DURATION_H_
#define EVNET_DURATION_H_

#include "common.h"

// A Duration represents the elapsed time between two instants
// as an int64 nanosecond count. The representation limits the
// largest representable duration to approximately 290 years.
class Duration {
  public:
    static const int64_t kNanosecond; // = 1LL
    static const int64_t kMicrosecond;// = 1000
    static const int64_t kMillisecond;// = 1000 * kMicrosecond
    static const int64_t kSecond; // = 1000 * kMillisecond
    static const int64_t kMinute; // = 60 * kSecond
    static const int64_t kHour;   // = 60 * kMinute

  public:
    Duration();
    explicit Duration(const struct timeval& t);
    explicit Duration(int64_t nanoseconds);
    explicit Duration(int nanoseconds);
    explicit Duration(double seconds);

    // Nanoseconds returns the duration as an integer nanosecond count.
    int64_t Nanoseconds() const;

    // These methods return double because the dominant
    // use case is for printing a floating point number like 1.5s, and
    // a truncation to integer would make them not useful in those cases.

    // Seconds returns the duration as a floating point number of seconds.
    double Seconds() const;

    double Milliseconds() const;
    double Microseconds() const;
    double Minutes() const;
    double Hours() const;

    struct timeval TimeVal() const;
    void To(struct timeval* t) const;

    bool IsZero() const;
    bool operator< (const Duration& rhs) const;
    bool operator<=(const Duration& rhs) const;
    bool operator> (const Duration& rhs) const;
    bool operator>=(const Duration& rhs) const;
    bool operator==(const Duration& rhs) const;

    Duration operator+=(const Duration& rhs);
    Duration operator-=(const Duration& rhs);
    Duration operator*=(int ns);
    Duration operator/=(int ns);

  private:
    int64_t nanoseconds_; // nanoseconds
};

inline Duration::Duration():
	nanoseconds_(0) 
{}

inline Duration::Duration(const struct timeval& t):
	nanoseconds_(t.tv_sec * kSecond + t.tv_usec * kMicrosecond) 
{}

inline Duration::Duration(int64_t nanoseconds):
	nanoseconds_(nanoseconds) 
{}

inline Duration::Duration(int nanoseconds):
	nanoseconds_(nanoseconds) 
{}

inline Duration::Duration(double seconds):
	nanoseconds_((int64_t)(seconds * kSecond))
{}

inline int64_t Duration::Nanoseconds() const {
	return nanoseconds_;
}

inline double Duration::Seconds() const {
	return double(nanoseconds_) / kSecond;
}

inline double Duration::Milliseconds() const {
	return double(nanoseconds_) / kMillisecond;
}

inline double Duration::Microseconds() const {
	return double(nanoseconds_) / kMicrosecond;
}

inline double Duration::Minutes() const {
	return double(nanoseconds_) / kMinute;
}

inline double Duration::Hours() const {
	return double(nanoseconds_) / kHour;
}

inline bool Duration::IsZero() const {
	return nanoseconds_ == 0;
}

inline struct timeval Duration::TimeVal() const {
	struct timeval t;
	To(&t);
	return t;
}

inline void Duration::To(struct timeval* t) const {
	t->tv_sec = (long)(nanoseconds_ / kSecond);
	t->tv_usec = (long)(nanoseconds_ % kSecond) / (long)kMicrosecond;
}

inline bool Duration::operator<(const Duration& rhs) const {
	return nanoseconds_ < rhs.nanoseconds_;
}

inline bool Duration::operator<=(const Duration& rhs) const {
	return nanoseconds_ <= rhs.nanoseconds_;
}

inline bool Duration::operator>(const Duration& rhs) const {
	return nanoseconds_ > rhs.nanoseconds_;
}

inline bool Duration::operator>=(const Duration& rhs) const {
	return nanoseconds_ >= rhs.nanoseconds_;
}

inline bool Duration::operator==(const Duration& rhs) const {
	return nanoseconds_ == rhs.nanoseconds_;
}

inline Duration Duration::operator+=(const Duration& rhs) {
	nanoseconds_ += rhs.nanoseconds_;
	return *this;
}

inline Duration Duration::operator-=(const Duration& rhs) {
	nanoseconds_ -= rhs.nanoseconds_;
	return *this;
}

inline Duration Duration::operator*=(int n) {
	nanoseconds_ *= n;
	return *this;
}

inline Duration Duration::operator/=(int n) {
	nanoseconds_ /= n;
	return *this;
}

#endif
