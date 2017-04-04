#include <unistd.h>
#include <errno.h>

#include "event_loop.h"
#include "socket.h"
#include "channel.h"
#include "tcp_connection.h"

TcpConnection::TcpConnection(EventLoop* loop, string name, int sockfd, const InetAddress& peeraddr):
  loop_(loop),
  name_(name),
  peeraddr_(peeraddr),
  socket_(new Socket(sockfd)),
  channel_(new Channel(loop, sockfd)),
  state_(kConnecting)
{
  socket_->SetNonblock();
  socket_->SetNagle(false);
  socket_->SetLinger(true);
  socket_->SetReuseAddr(true);
  socket_->SetKeepAlive(true);

  channel_->set_read_callback(bind(&TcpConnection::HandleReadEvent, this));
  channel_->set_write_callback(bind(&TcpConnection::HandleWriteEvent, this));
  channel_->set_close_callback(bind(&TcpConnection::HandleCloseEvent, this));
  channel_->set_error_callback(bind(&TcpConnection::HandleErrorEvent, this));
}

TcpConnection::~TcpConnection() {
}

void TcpConnection::ConnectionEstablished() {
  CHECK(loop_->IsInLoopThread());
  channel_->EnableReadEvent();
  state_ = kConnected;

  if (connection_callback_) {
    connection_callback_(shared_from_this());
  }
}

void TcpConnection::ConnectionDestroyed() {
  CHECK(loop_->IsInLoopThread());
  if (kConnected == state_) {
    state_ = kDisconnected;
    channel_->DisableAllEvent();

    // recall connection_callback_
    // for handle close connection
    if (connection_callback_) {
      connection_callback_(shared_from_this());
    }
  }

  channel_->Remove();
}

void TcpConnection::Send(const void* message, int len) {
  if (kConnected != state_) {
    return ;
  }

  if (loop_->IsInLoopThread()) {
    SendInLoop(message, len);
  } else {
    loop_->RunInLoop(bind(&TcpConnection::SendInLoop, this, message, len));
  }
}

void TcpConnection::Send(const string& message) {
  Send(message.c_str(), message.size());
}

void TcpConnection::SendInLoop(const void* message, int len) {
  CHECK(loop_->IsInLoopThread());
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
        LOG(ERROR) << "TcpConnection send error";
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

void TcpConnection::Shutdown() {
  if (kConnected == state_) {
    state_ = kDisconnecting;

    if (!channel_->HasWriteEvent()) {
      socket_->ShutdownWrite();
    }
  }
}

const char* TcpConnection::StateToString() const {
  switch (state_) {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

void TcpConnection::set_connection_callback(const ConnectionCallback& cb) {
  connection_callback_ = cb;
}

void TcpConnection::set_message_callback(const MessageCallback& cb) {
  message_callback_ = cb;
}

void TcpConnection::set_write_complete_callback(const WriteCompleteCallback& cb) {
  write_complete_callback_ = cb;
}

void TcpConnection::set_close_callback(const CloseCallback& cb) {
  close_callback_ = cb;
}

void TcpConnection::HandleReadEvent() {
  CHECK(loop_->IsInLoopThread());
  int n = 0;
  int _errno;
  n = input_buffer_.ReadFd(socket_->fd(), _errno);

  if (n > 0) {
    if (message_callback_) {
      message_callback_(shared_from_this(), input_buffer_);
    }
  } else if (0 == n) {
    HandleCloseEvent();
  } else {
    HandleErrorEvent();
  }
}

void TcpConnection::HandleWriteEvent() {
  CHECK(loop_->IsInLoopThread());
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
        LOG(ERROR) << "TcpConnection handle write error";
      }
    }
  }
}

void TcpConnection::HandleCloseEvent() {
  CHECK(loop_->IsInLoopThread());
  channel_->DisableAllEvent();

  if (close_callback_) {
    close_callback_(shared_from_this());
  } 
}

void TcpConnection::HandleErrorEvent() {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);
  int err = 0;

  if (::getsockopt(socket_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    err = errno;
  } else {
    err = optval;
  }

  LOG(ERROR) << "TcpConnection handle error: name=" << name_ << ", SO_ERROR=" << strerror(err);
}
