#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>

#include <boost/shared_ptr.hpp>

#include "eventloop.h"
#include "inetaddress.h"
#include "connector.h"
#include "connection.h"
#include "log.h"

using std::map;
using boost::shared_ptr;

EventLoop g_loop;
map<string, shared_ptr<Connection> > g_connections;
int g_next_connection_id = 1;

void connection_callback(const Connection* connection) {
    log_info("connection callback: new connection [%s] from %s:%d", \
            connection->name().c_str(), connection->peeraddr().ip().c_str(), \
            connection->peeraddr().port());
}

void message_callback(const Connection* connection, const char* buf, int len) {
    (void)buf;
    log_info("message callback: received %d bytes from connection [%s]", \
            len, connection->name().c_str());
}

void close_callback(const Connection* connection) {
    log_info("close callback: remote peer[%s] closed connection", \
            connection->name().c_str());

    // if server occur internal error, loop quit.
    g_loop.quit();
}

void new_connection(int sockfd, const InetAddress& peeraddr) {
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", g_next_connection_id);
    ++g_next_connection_id;
    string connection_name = buf;

    log_info("Established a new connection %s with %s:%d", connection_name.c_str(), \
            peeraddr.ip().c_str(), peeraddr.port());

    shared_ptr<Connection> connection(new Connection(&g_loop, connection_name, \
                sockfd, peeraddr));
    g_connections[connection_name] = connection;
    connection->set_connection_callback(connection_callback);
    connection->set_message_callback(message_callback);
    connection->set_close_callback(close_callback);
    connection->connectionEstablished();

    // send message "hello, world"
    connection->send("hello, world", 12);
}

int main() {
    InetAddress server_addr("localhost", 12345);
    Connector connector(server_addr);

    connector.set_new_connection_callback(new_connection);
    int num_clients = 5;

    for (int i = 0; i < num_clients; i++) {
        connector.start();
    }

    g_loop.loop();

    return 0;
}
