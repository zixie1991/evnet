#ifndef PTIME_H_
#define PTIME_H_

#include <stdint.h>

#include <string>

class Timestamp {
    public:
        explicit Timestamp(long microseconds);
        ~Timestamp();

        void now();

        long seconds() const {
            return microseconds_ / kMicrosecondsPerSecond;
        }

        long milliseconds() const {
            return microseconds_ / kMillisecondsPerSecond;
        }

        long microseconds() const {
            return microseconds_;
        }

        std::string toString() const;
        std::string toSimpleString() const;

    private:
        const int kMicrosecondsPerSecond = 1000000;
        const int kMillisecondsPerSecond = 1000;

        // microseconds since the Epoch
        long microseconds_;
};

inline bool operator<(const Timestamp& lhs, const Timestamp& rhs) {
    return lhs.microseconds() < rhs.microseconds();
}

inline bool operator==(const Timestamp& lhs, const Timestamp& rhs) {
    return lhs.microseconds() - rhs.microseconds();
}

#endif // PTIME_H_
