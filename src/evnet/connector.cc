#include <fcntl.h>
#include <errno.h>

#include "socket.h"
#include "channel.h"
#include "connector.h"

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
    LOG(ERROR) << "Create socket error: " << strerror(errno);
    return ;
  }

  int ret = 0;
  ret = ::connect(sockfd_, (struct sockaddr *)&server_addr_.addr(), sizeof(server_addr_.addr()));
  if (ret < 0) {
    LOG(ERROR) << "Connect to server " << server_addr_.ip() << ":" << server_addr_.port() << " error: " << strerror(errno);
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
