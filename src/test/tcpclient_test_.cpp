#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <boost/shared_ptr.hpp>

#include "eventloop.h"
#include "inetaddress.h"
#include "tcpclient.h"
#include "log.h"

using boost::shared_ptr;

void connection_callback(const shared_ptr<Connection>& connection) {
    log_info("connection callback: new connection [%s] to %s:%d", \
            connection->name().c_str(), connection->peeraddr().ip().c_str(), \
            connection->peeraddr().port());
}

void message_callback(const shared_ptr<Connection>& connection, Buffer& buffer) {
    (void)buffer;
    log_info("message callback: received %d bytes from connection [%s]", \
            buffer.readableBytes(), connection->name().c_str());
}

int main() {
    set_log_level(Logger::LEVEL_TRACE);
    EventLoop loop;
    InetAddress server_addr("localhost", 12345);

    TcpClient client(&loop, server_addr);
    client.set_connection_callback(connection_callback);
    client.set_message_callback(message_callback);

    client.connect();

    loop.loop();
    return 0;
}
