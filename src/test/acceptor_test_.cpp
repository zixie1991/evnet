#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inetaddress.h"
#include "eventloop.h"
#include "acceptor.h"
#include "log.h"

void new_connection(int sockfd, const InetAddress& peeraddr) {
    log_info("Accepted a new connection from %s:%d", peeraddr.ip().c_str(), peeraddr.port());
    ::close(sockfd);
}

int main() {
    InetAddress listen_addr(12345);
    EventLoop loop;

    Acceptor acceptor(&loop, listen_addr);
    acceptor.set_new_connection_callback_(new_connection);
    acceptor.listen();

    loop.loop();
    return 0;
}
