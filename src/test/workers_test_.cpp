#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

#include "eventloop.h"
#include "thread.h"
#include "workers.h"
#include "tcpserver.h"
#include "log.h"

using boost::shared_ptr;
using boost::bind;
using boost::any_cast;
using std::string;

Workers* g_workers = NULL;

class LogWorker: public Worker {
    public:
        LogWorker() {

        }
        ~LogWorker() {

        }

        virtual int preProcess() {
            log_info("before process");
            return 0;
        }

        virtual int process() {
            log_info("receive data: %s", request_.c_str());
            response_ = request_.c_str();
            return 0;
        }

        virtual int postProcess() {
            log_info("after process");
            return 0;
        }

        virtual int callback() {
            connection_->send(response_.c_str(), response_.size());            
            return 0;
        }

        void set_connection(const shared_ptr<Connection>& connection) {
            connection_ = connection;
        }

        void set_request(string request) {
            request_ = request;
        }
    private:
        shared_ptr<Connection> connection_;
        string request_;
        string response_;
};

void connection_callback(const shared_ptr<Connection>& connection) {
    log_info("connection callback: new connection [%s] from %s:%d", \
            connection->name().c_str(), connection->peeraddr().ip().c_str(), \
            connection->peeraddr().port());
}

void message_callback(const shared_ptr<Connection>& connection, Buffer& buffer) {
    (void)buffer;
    log_info("message callback: received %d bytes from connection [%s]", \
            buffer.readableBytes(), connection->name().c_str());

    string message;
    buffer.read(message);

    LogWorker* worker = new LogWorker();
    worker->set_connection(connection);
    worker->set_request(message);

    g_workers->delegate(worker); 
}

int main() {
    set_log_level(Logger::LEVEL_WARN);
    EventLoop loop;
    InetAddress listen_addr(12345);

    WorkerPool workerpool;
    workerpool.start(10);

    g_workers = new Workers(&loop, &workerpool);

    TcpServer server(&loop, listen_addr);
    server.set_connection_callback(connection_callback);
    server.set_message_callback(message_callback);

    server.start();

    return 0;
}
