#include "evnet/common.h"
#include "evnet/tcp_client.h"
#include "evnet/event_loop.h"

class EchoClient;

class Session {
  public:
    Session(EventLoop* loop, const InetAddress& server_addr, EchoClient* owner):
      client_(loop, server_addr),
      owner_(owner)
    {
      client_.set_connection_callback(bind(&Session::OnConnection, this, _1));
      client_.set_message_callback(bind(&Session::OnMessage, this, _1, _2));
    }

    void Start() {
      client_.Connect();
    }

    void Stop() {
      client_.Disconnect();
    }

  private:
    void OnConnection(const shared_ptr<TcpConnection>& connection);
    void OnMessage(const shared_ptr<TcpConnection>& connection, Buffer& buffer);

    TcpClient client_;
    EchoClient* owner_;
};

class EchoClient {
  public:
    EchoClient(EventLoop* loop, InetAddress server_addr, int session_num , int data_size):
      loop_(loop),
      message_(data_size, 'a')
    {
      for (int i = 0; i < session_num; i++) {
        shared_ptr<Session> session(new Session(loop, server_addr, this));
        sessions_.push_back(session);
        session->Start();
      }
    }

    ~EchoClient() {
    }

    const string& message() const {
      return message_;
    }

  private:
    EventLoop *loop_;
    vector<shared_ptr<Session>> sessions_;

    string message_;
};

int main(int argc, char *argv[]) {
  if (5 != argc) {
    fprintf(stderr, "Usage: %s <ip> <port> <num_clients> <data_size>\n", argv[0]);
    exit(0);
  }

  string ip(argv[1]);
  uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
  int num_clients = atoi(argv[3]);
  int data_size = atoi(argv[4]);
  InetAddress server_addr(ip, port);

  EventLoop loop;
  EchoClient echo_client(&loop, server_addr, num_clients, data_size);
  loop.Run();

  return 0;
}

void Session::OnConnection(const shared_ptr<TcpConnection>& connection) {
  if (connection->connected()) {
    // send message "hello, world"
    connection->Send(owner_->message().data(), owner_->message().size());
  }
}

void Session::OnMessage(const shared_ptr<TcpConnection>& connection, Buffer& buffer) {
  string message;
  buffer.Read(message);
  connection->Send(message);
}
