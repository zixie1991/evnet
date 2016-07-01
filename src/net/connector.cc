#include "connector.h"

#include <fcntl.h>
#include <errno.h>

#include <boost/bind.hpp>

#include "util/log.h"

#include "socket.h"
#include "channel.h"

Connector::Connector(const InetAddress& server_addr):
  server_addr_(server_addr),
  sockfd_(-1),
  connect_(false)
{
}

Connector::~Connector() {

}

void Connector::Start() {
  connect_ = true; 
  Connect();
}

void Connector::Restart() {

}

void Connector::Stop() {

}

void Connector::Connect() {
  if ((sockfd_ = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
  }

  int ret = 0;
  ret = ::connect(sockfd_, (struct sockaddr *)&server_addr_.addr(), sizeof(server_addr_.addr()));
  if (ret < 0) {
    log_error("connect to server %s:%d error", server_addr_.ip().c_str(), \
            server_addr_.port());
  } else {
    Connecting();
  }
}

void Connector::Connecting() {
  new_connection_callback_(sockfd_, server_addr_);
}

void Connector::set_new_connection_callback(const NewConnectionCallback& cb) {
  new_connection_callback_ = cb;
}
