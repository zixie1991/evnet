#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdio.h>
#include <stdlib.h>

#include <vector>

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
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize=kInitialSize):
            buffer_(0 + initialSize),
            reader_index_(0),
            writer_index_(0)
        {}

        /**
         * @brief 可读的字节数目
         */
        int readableBytes();

        /**
         * @brief 可写的字节数目
         */
        int writableBytes();

        /**
         * @brief 丢弃已读的内容
         */
        int discardReadBytes();

        /**
         * @brief 保证可写的字节数目要大于等于将要写入的字节数目
         */
        void ensureWritableBytes(int len);

        void set_reader_index(int reader_index);

        int reader_index();

        void set_writer_index(int writer_index);

        int writer_index();

        /**
         * @brief 从reader_index_独处指定数目的字节，同时reader_index_=reader_index_+byteNumber.
         */
        std::string read(int len);
        std::string read();
        int64_t readInt64();
        int32_t readInt32();
        int16_t readInt16();
        int8_t readInt8();

        /**
         * @brief 写入指定数目的字节，同时writer_index_=writer_index_ + byteNumber. 如果
         *  wirterIndex_ > capacity，buffer_会自动增长
         */
        void write(const char* data, int len);
        void write(const void* data, int len);
        void writeInt64(int64_t x);
        void writeInt32(int32_t x);
        void writeInt16(int16_t x);
        void writeInt8(int8_t x);

        /**
         * @brief capacity of buffer.
         */
        int capacity() const {
            return buffer_.capacity();
        }

    private:
        /**
         * @brief buffer_首地址
         */
        char* begin() {
            return &*buffer_.begin();
        }

        /**
         * @brief buffer_尾部地址
         */
        const char* end() {
            return &*buffer_.end();
        }

        /**
         * @brief buffer_自动增长
         */
        void makeSpace(size len);

        // 一块连续的内存
        std::vector<char> buffer_;
        // 控制读缓冲区首地址
        int reader_index_;
        // 控制写缓冲区首地址
        int writer_index_;
};

#endif // BUFFER_H_
