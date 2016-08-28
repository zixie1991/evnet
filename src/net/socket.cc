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

#include "util/log.h"

#include "inetaddress.h"

Socket::~Socket() {
  log_debug("Socket desctrutor: close socket");
  ::close(sockfd_);
}

int Socket::GenerateSocket() {
  if (-1 != sockfd_) {
    return sockfd_;
  }

  if ((sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  return sockfd_;
}

void Socket::BindAddress(const InetAddress& localaddr) {
  int ret = ::bind(sockfd_, (struct sockaddr *)&localaddr.addr(), sizeof(localaddr.addr()));
  
  if (-1 == ret) {
    log_error("Bind a name to a socket error");
    abort();
  }
}

void Socket::Listen() {
  if (::listen(sockfd_, SOMAXCONN) < 0) {
    log_error("Listen for connection error");
    abort();
  }
}

int Socket::Accept(InetAddress &peeraddr) {
  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  memset(&addr, 0, sizeof(addr));

  int fd = ::accept(sockfd_, (struct sockaddr *)&addr, &addr_len);
  peeraddr.set_addr(addr);    

  return fd;
}

void Socket::SetNagle(bool on) {
  int val = on ? 0 : 1;

  if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, (const void *)&val, sizeof(val)) < 0) {
    log_error("%s Nagle algorithm error", on ? "enable" : "disable");
  }
}

//使用SO_LINGER，主动close后不进入TIME_WAIT状态
void Socket::SetLinger(bool on) {
  struct linger ling = {0, 0};
  ling.l_onoff = on ? 1: 0;

  if (::setsockopt(sockfd_, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) < 0) {
    log_error("%s Linger error", on ? "Enable" : "Disable");
  }
}

void Socket::SetReuseAddr(bool on) {
  int val = on ? 1 : 0;

  if (::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&val, sizeof(val)) < 0) {
    log_error("%s reuse address error", on ? "Enable" : "Disable");
  }
}

void Socket::SetNonblock() {
  int flag;

  if ((flag = ::fcntl(sockfd_, F_GETFL, 0)) < 0) {
    log_error("Access mode and the file status flags error");
  }

  flag |= O_NONBLOCK;

  if (::fcntl(sockfd_, F_SETFL, flag) < 0) {
    log_error("Set file status flags(O_NONBLOCK) error");
  }
}

void Socket::ShutdownWrite() {
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    log_error("Close socket on write error");
  }
}

int Socket::Connect(InetAddress& peeraddr) {
    int ret = 0;
    ret = ::connect(sockfd_, (struct sockaddr *)&peeraddr.addr(), sizeof(peeraddr.addr()));

    return ret;
}

// 周期性的测试连接是否存活
void Socket::SetKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  if (::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval)) < 0) {
    log_error("%s KeepAlive error", on ? "Enable": "Disable");
  }
}
