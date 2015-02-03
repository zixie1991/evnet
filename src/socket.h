#ifndef SOCKET_H_
#define SOCKET_H_

// forward declartion
class InetAddress;

class Socket {
    public:
        explicit Socket(int sockfd):
            sockfd_(sockfd)
        {}

        ~Socket();

        int fd() const { return sockfd_; }

        /**
         * @brief abort if address in use
         */
        void bindAddress(const InetAddress& localaddr);

        /**
         * @brief abort if address in use
         */
        void listen();

        /**
         * @brief
         * @retval On success, return a non-negative integer that is a 
         *  descriptor for the accepted socket
         *  On error, -1 is returned
         */
        int accept(InetAddress& peeraddr);

        // disable Nagle Aglorithm
        void disableNagle();

        // disable Linger(取消延迟关闭)
        void disableLinger();

        // Enable/disable reuseAddr(短连接)
        void setReuseAddr(bool on);

        /**
         * @brief Enable/disable SO_KEEPALIVE
         */
        void setKeepAlive(bool on);

    private:
        int sockfd_;
};

#endif // SOCKET_H_
