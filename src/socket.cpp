#include "socket.h"

#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "inetaddress.h"
#include "log.h"

Socket::~Socket() {
    ::close(sockfd_);
}

int Socket::generateSocket() {
    if (-1 != sockfd_) {
        return sockfd_;
    }

    if ((sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    return sockfd_;
}

void Socket::bindAddress(const InetAddress& localaddr) {
    int ret = ::bind(sockfd_, (struct sockaddr *)&localaddr.addr(), sizeof(localaddr.addr()));
    
    if (-1 == ret) {
        log_error("bind a name to a socket error");
        abort();
    }
}

void Socket::listen() {
    if (::listen(sockfd_, SOMAXCONN) < 0) {
        log_error("listen for connection error");
        abort();
    }
}

int Socket::accept(InetAddress &peeraddr) {
    struct sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    memset(&addr, 0, sizeof(addr));

    int fd = ::accept(sockfd_, (struct sockaddr *)&addr, &addrLen);
    peeraddr.set_addr(addr);    

    return fd;
}

void Socket::setNagle(bool on) {
    int val = on ? 0 : 1;

    if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, (const void *)&val, sizeof(val)) < 0) {
        log_error("%s Nagle algorithm error", on ? "enable" : "disable");
    }
}


void Socket::setLinger(bool on) {
    struct linger ling = {0, 0};
    ling.l_onoff = on ? 1: 0;

    if (::setsockopt(sockfd_, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) < 0) {
        log_error("%s Linger error", on ? "enable" : "disable");
    }
}

void Socket::setReuseAddr(bool on) {
    int val = on ? 1 : 0;

    if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&val, sizeof(val)) < 0) {
        log_error("%s reuse address error", on ? "enable" : "disable");
    }
}

void Socket::setNonblock() {
    int flag;

    if ((flag = ::fcntl(sockfd_, F_GETFL, 0)) < 0) {
        log_error("access mode and the file status flags error");
    }

    flag |= O_NONBLOCK;

    if (::fcntl(sockfd_, F_SETFL, flag) < 0) {
        log_error("set file status flags(O_NONBLOCK) error");
    }
}

void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        log_error("close socket on write error");
    }
}

int Socket::connect(InetAddress& peeraddr) {
    int ret = 0;
    ret = ::connect(sockfd_, (struct sockaddr *)&peeraddr.addr(), sizeof(peeraddr.addr()));

    return ret;
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, 
            &optval, static_cast<socklen_t>(sizeof optval));  
}
