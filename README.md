Linux C++ 网络编程
====

**公共接口**

*   Buffer 仿 Netty ChannelBuffer 的 buffer class，数据的读写透过 buffer 进行
*   InetAddress 封装 IPv4 地址 (end point)，注意，muduo 目前不能解析域名，只认 IP
*   EventLoop 反应器 Reactor，用户可以注册计时器回调
*   EventLoopThread 启动一个线程，在其中运行 EventLoop::loop()
*   Connection 整个网络库的核心，封装一次 TCP 连接
*   TcpClient 用于编写网络客户端，能发起连接，并且有重试功能
*   TcpServer 用于编写网络服务器，接受客户的连接
*   在这些类中，Connection 的生命期依靠 shared_ptr 控制（即用户和库共同控制）。Buffer 的生命期由 Connection 控制。其余类的生命期由用户控制。

**内部实现**


*   Channel 是 selectable IO channel，负责注册与响应 IO 事件，它不拥有 file descriptor。它是 Acceptor、Connector、EventLoop、TimerQueue、Connection 的成员，生命期由后者控制。
*   Socket 封装一个 file descriptor，并在析构时关闭 fd。它是 Acceptor、Connection 的成员，生命期由后者控制。EventLoop、TimerQueue 也拥有 fd，但是不封装为 Socket。
*   SocketsOps 封装各种 sockets 系统调用。
*   EventLoop 封装事件循环，也是事件分派的中心。它用 eventfd(2) 来异步唤醒，这有别于传统的用一对 pipe(2) 的办法。它用 TimerQueue 作为计时器管理，用 Poller 作为 IO Multiplexing。
*   Poller 是 PollPoller 和 EPollPoller 的基类，采用“电平触发”的语意。它是 EventLoop 的成员，生命期由后者控制。
*   PollPoller 和 EPollPoller 封装 poll(2) 和 epoll(4) 两种 IO Multiplexing 后端。Poll 的存在价值是便于调试，因为 poll(2) 调用是上下文无关的，用 strace 很容易知道库的行为是否正确。
*   Connector 用于发起 TCP 连接，它是 TcpClient 的成员，生命期由后者控制。
*   Acceptor 用于接受 TCP 连接，它是 TcpServer 的成员，生命期由后者控制。
*   TimerQueue 用 timerfd 实现定时，这有别于传统的设置 poll/epoll_wait 的等待时长的办法。为了简单起见，目前用链表来管理 Timer，如果有必要可改为优先队列，这样复杂度可从 O(n) 降为O(ln n) （某些操作甚至是 O(1)）。它是 EventLoop 的成员，生命期由后者控制。
*   EventLoopThreadPool 用于创建 IO 线程池，也就是说把 Connection 分派到一组运行 EventLoop 的线程上。它是 TcpServer 的成员，生命期由后者控制。

