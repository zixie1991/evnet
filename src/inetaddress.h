#ifndef NETADDRESS_H_
#define NETADDRESS_H_

#include <netinet/in.h>

#include <stdint.h>

#include <string>

class InetAddress {
    public:
        explicit InetAddress(uint16_t port=0);
        InetAddress(const std::string& ip, uint16_t port);
        InetAddress(const struct sockaddr_in& addr):
            addr_(addr)
        {
        }

        std::string ip() const;
        uint16_t port() const;

        const struct sockaddr_in& addr() const {
            return addr_;
        }

        void set_addr(const struct sockaddr_in& addr) {
            addr_ = addr;
        }

    private:
        struct sockaddr_in addr_;
};

#endif // NETADDRESS_H_
