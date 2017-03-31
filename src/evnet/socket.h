#ifndef EVNET_SOCKET_H_
#define EVNET_SOCKET_H_

// forward declartion
class InetAddress;

class Socket {
  public:
    explicit Socket(int sockfd=-1):
        sockfd_(sockfd)
    {}

    ~Socket();

    int fd() const { return sockfd_; }

    // generate socket
    int GenerateSocket();

    /**
      * @brief abort if address in use
      */
    void BindAddress(const InetAddress& localaddr);

    /**
      * @brief abort if address in use
      */
    void Listen();

    /**
      * @brief
      * @retval On success, return a non-negative integer that is a 
      *  descriptor for the accepted socket
      *  On error, -1 is returned
      */
    int Accept(InetAddress& peeraddr);

    int Connect(InetAddress& peeraddr);

    void ShutdownWrite();

    /**
      * @brief set socket nonblock
      */
    void SetNonblock();

    /**
      * @brief Enable/disable Nagle Aglorithm
      */
    void SetNagle(bool on);

    /**
      * @breif Enable/disable Linger(取消延迟关闭)
      */
    void SetLinger(bool on);

    /**
      * @brief Enable/disable reuseAddr(短连接)
      */
    void SetReuseAddr(bool on);

    /**
      * @brief Enable/disable SO_KEEPALIVE
      */
    void SetKeepAlive(bool on);

  private:
    int sockfd_;
};

#endif
