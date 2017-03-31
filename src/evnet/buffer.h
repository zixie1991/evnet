#ifndef EVNET_BUFFER_H_
#define EVNET_BUFFER_H_

#include "common.h"

/**
 * @brief 应用层缓冲区: a buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
 * @code
 * +-------------------+------------------+------------------+
 * | discardable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 *
 * 0      <=      reader_index   <=   writer_index    <=    capacity
 * @endcode
 */
class Buffer {
  public:
    // buffer_ 初始化长度
    static const int kInitialSize = 1024;

    explicit Buffer(size_t initialSize=kInitialSize):
        buffer_(0 + initialSize),
        reader_index_(0),
        writer_index_(0)
    {}

    ~Buffer() {}

    /**
      * @brief 可读的字节数目
      */
    int ReadableBytes();

    /**
      * @brief 可写的字节数目
      */
    int WritableBytes();

    /**
      * @brief 丢弃已读的内容
      */
    int DiscardReadBytes();

    /**
      * @brief 保证可写的字节数目要大于等于将要写入的字节数目
      */
    void EnsureWritableBytes(int len);

    void set_reader_index(int reader_index) {
        reader_index_ = reader_index;
    }

    int reader_index() {
        return reader_index_;
    }

    void set_writer_index(int writer_index) {
        writer_index_ = writer_index;
    }

    int writer_index() {
        return writer_index_;
    }

    /**
      * @brief 从reader_index_独处指定数目的字节，同时reader_index_=reader_index_+byte_number.
      * read from network to host
      */
    int Read(char* data, int len);
    int Read(char* data);
    int Read(void* data, int len);
    int Read(void* data);
    int Read(string& data, int len);
    int Read(string& data);
    int ReadInt64(int64_t& x);
    int ReadInt32(int32_t& x);
    int ReadInt16(int16_t& x);
    int ReadInt8(int8_t& x);

    /**
      * @brief 写入指定数目的字节，同时writer_index_=writer_index_ + byteNumber. 如果
      * wirterIndex_ > capacity，buffer_会自动增长
      * write from host to network
      */
    void Write(const char* data, int len);
    void Write(const void* data, int len);
    void Write(const string& data);
    void WriteInt64(int64_t x);
    void WriteInt32(int32_t x);
    void WriteInt16(int16_t x);
    void WriteInt8(int8_t x);

    /**
      * @brief capacity of buffer.
      */
    int Capacity() const {
        return buffer_.capacity();
    }

    /**
      * @brief Read data from socket into buffer.
      * @return result of read(2), @c errno is saved
      */
    int ReadFd(int fd, int& _errno);

    /**
      * @brief just move reader_index_, not read data
      */
    int Retrieve(int len);

    char* Peek();
  private:
    /**
      * @brief buffer_首地址
      */
    char* Begin() {
      return &*buffer_.begin();
    }

    /**
      * @brief buffer_尾部地址
      */
    const char* End() {
      return &*buffer_.end();
    }

    /**
      * @brief buffer_自动增长
      */
    void MakeSpace(int len);

    // 一块连续的内存
    vector<char> buffer_;
    // 控制读缓冲区首地址
    int reader_index_;
    // 控制写缓冲区首地址
    int writer_index_;
};

#endif
