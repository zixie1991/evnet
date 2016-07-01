#include "connection.h"

#include <errno.h>
#include <string.h>

#include <boost/bind.hpp>

#include "util/log.h"

#include "socket.h"
#include "channel.h"

using boost::bind;

Connection::Connection(EventLoop* loop, std::string name, int sockfd, const InetAddress& peeraddr):
  loop_(loop),
  name_(name),
  peeraddr_(peeraddr),
  socket_(new Socket(sockfd)),
  channel_(new Channel(loop, sockfd)),
  state_(kConnecting)
{
  log_debug("Connection constructor: name:%s, fd:%d", name_.c_str(), socket_->fd());
  socket_->SetNonblock();
  socket_->SetNagle(false);
  socket_->SetLinger(true);
  socket_->SetReuseAddr(true);
  socket_->SetKeepAlive(true);

  channel_->set_read_callback(boost::bind(&Connection::HandleReadEvent, this));
  channel_->set_write_callback(boost::bind(&Connection::HandleWriteEvent, this));
  channel_->set_close_callback(boost::bind(&Connection::HandleCloseEvent, this));
  channel_->set_error_callback(boost::bind(&Connection::HandleErrorEvent, this));
}

Connection::~Connection() {
  log_debug("Connection destructor: name=%s, fd=%d", name_.c_str(), socket_->fd());
}

void Connection::ConnectionEstablished() {
  channel_->EnableReadEvent();
  state_ = kConnected;

  connection_callback_(shared_from_this());
}

void Connection::ConnectionDestroyed() {
  if (kConnected == state_) {
    state_ = kDisconnected;
    channel_->DisableAllEvent();

    // recall connection_callback_
    connection_callback_(shared_from_this());
  }

  channel_->Remove();
}

void Connection::Send(const void* message, int len) {
  if (kConnected != state_) {
    return ;
  }

  if (loop_->IsInLoopThread()) {
    SendInLoop(message, len);
  } else {
    loop_->RunInLoop(bind(&Connection::SendInLoop, this, message, len));
  }
}

void Connection::SendInLoop(const void* message, int len) {
  if (kConnected != state_) {
    return ;
  }

  int n = 0;
  bool fault_error = false;

  // if no thing on output buffer, try write directly.
  if (!channel_->HasWriteEvent() && 0 == output_buffer_.ReadableBytes()) {
    n = ::write(socket_->fd(), message, len);
    (void)n;

    if (n >= 0) {
      // FIXME write data complete.
      if (n == len && write_complete_callback_) {
        loop_->QueueInLoop(bind(write_complete_callback_, shared_from_this()));
      }
    } else {
      n = 0;
      if (EWOULDBLOCK != errno && EAGAIN != errno) {
        log_error("Connection send error");
        if (EPIPE == errno || ECONNRESET == errno) {
            fault_error = true;
        }
      }
    }
  }

  if (!fault_error && n < len) {
    output_buffer_.Write(static_cast<const char *>(message) + n, len - n);
    if (!channel_->HasWriteEvent()) {
      channel_->EnableWriteEvent();
    }
  }
}

void Connection::Shutdown() {
  if (kConnected == state_) {
    state_ = kDisconnecting;

    if (!channel_->HasWriteEvent()) {
      socket_->ShutdownWrite();
    }
  }
}

void Connection::set_connection_callback(const ConnectionCallback& cb) {
  connection_callback_ = cb;
}

void Connection::set_message_callback(const MessageCallback& cb) {
  message_callback_ = cb;
}

void Connection::set_write_complete_callback(const WriteCompleteCallback& cb) {
  write_complete_callback_ = cb;
}

void Connection::set_close_callback(const CloseCallback& cb) {
  close_callback_ = cb;
}

void Connection::HandleReadEvent() {
  int n = 0;
  int _errno;
  n = input_buffer_.ReadFd(socket_->fd(), _errno);

  if (n > 0) {
    log_debug("handle read event: received %d bytes", n);
    message_callback_(shared_from_this(), input_buffer_);
  } else if (0 == n) {
    HandleCloseEvent();
  } else {
    HandleErrorEvent();
  }
}

void Connection::HandleWriteEvent() {
  if (channel_->HasWriteEvent()) {
    int n = 0;
    n = ::write(socket_->fd(), output_buffer_.Peek(), output_buffer_.ReadableBytes());
    int readable = output_buffer_.ReadableBytes();
    (void)readable;

    if (n > 0) {
      output_buffer_.Retrieve(n);

      if (0 == output_buffer_.ReadableBytes()) {
        channel_->DisableWriteEvent();

        if (write_complete_callback_) {
          loop_->QueueInLoop(bind(write_complete_callback_, shared_from_this()));
        }
      }
    } else {

      if (EWOULDBLOCK != errno && EAGAIN != errno) {
        log_error("Connection handle write error");
      }
    }
  }
}

void Connection::HandleCloseEvent() {
  channel_->DisableAllEvent();

  close_callback_(shared_from_this());
}

void Connection::HandleErrorEvent() {
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
