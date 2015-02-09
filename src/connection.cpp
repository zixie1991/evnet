#include "connection.h"

#include <errno.h>
#include <string.h>

#include <boost/bind.hpp>

#include "socket.h"
#include "channel.h"
#include "log.h"


Connection::Connection(EventLoop* loop, std::string name, int sockfd, const InetAddress& peeraddr):
    loop_(loop),
    name_(name),
    peeraddr_(peeraddr),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd))
{
    log_debug("Connection constructor: name:%s, fd:%d", name_.c_str(), socket_->fd());
    socket_->setNonblock();
    socket_->setNagle(false);
    socket_->setLinger(true);
    socket_->setReuseAddr(true);
    socket_->setKeepAlive(true);

    channel_->set_read_callback(boost::bind(&Connection::handleReadEvent, this));
    channel_->set_write_callback(boost::bind(&Connection::handleWriteEvent, this));
    channel_->set_close_callback(boost::bind(&Connection::handleCloseEvent, this));
    channel_->set_error_callback(boost::bind(&Connection::handleErrorEvent, this));
}

Connection::~Connection() {
    log_debug("Connection destructor: name=%s, fd=%d", name_.c_str(), socket_->fd());
}

void Connection::connectionEstablished() {
    channel_->enableReadEvent();
    connection_callback_(shared_from_this());
}

void Connection::connectionStreamed() {
    channel_->enableWriteEvent();
}

void Connection::send(const void* message, int len) {
    int n = 0;
    if (!channel_->hasWriteEvent() && 0 == output_buffer_.readableBytes()) {
        n = ::write(socket_->fd(), message, len);
        (void)n;

        if (n >= 0) {
            // FIXME write data complete.
        } else {
            n = 0;
            if (EWOULDBLOCK != errno && EAGAIN != errno) {
                log_error("Connection send error");
            }
        }
    }

    if (n < len) {
        output_buffer_.write(static_cast<const char *>(message) + n, len - n);
        if (!channel_->hasWriteEvent()) {
            channel_->enableWriteEvent();
        }
    }
}

void Connection::set_connection_callback(const ConnectionCallback& cb) {
    connection_callback_ = cb;
}

void Connection::set_message_callback(const MessageCallback& cb) {
    message_callback_ = cb;
}

void Connection::set_close_callback(const CloseCallback& cb) {
    close_callback_ = cb;
}

void Connection::handleReadEvent() {
    int n = 0;
    int _errno;
    n = input_buffer_.readFd(socket_->fd(), _errno);

    if (n > 0) {
        log_debug("handle read event: received %d bytes", n);
        message_callback_(shared_from_this(), input_buffer_);
    } else if (0 == n) {
        handleCloseEvent();
    } else {
        handleErrorEvent();
    }
}

void Connection::handleWriteEvent() {
    if (channel_->hasWriteEvent()) {
        int n = 0;
        n = ::write(socket_->fd(), output_buffer_.peek(), output_buffer_.readableBytes());
        int readable = output_buffer_.readableBytes();

        if (n > 0) {
            output_buffer_.retrieve(n);

            if (0 == output_buffer_.readableBytes()) {
                channel_->disableWriteEvent();
            }
        } else {

            if (EWOULDBLOCK != errno && EAGAIN != errno) {
                log_error("Connection handle write error");
            }
        }
    }
}

void Connection::handleCloseEvent() {
    channel_->disableAllEvent();

    close_callback_(shared_from_this());
}

void Connection::handleErrorEvent() {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    int err = 0;

    if (::getsockopt(socket_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }

    log_error("Connection handle error: name=%s, SO_ERROR=%s", name_.c_str(), strerror(err));
}
