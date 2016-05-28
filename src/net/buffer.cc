#include "buffer.h"

#include <sys/uio.h>
#include <errno.h>
#include <endian.h>

#include <string.h>

#include <algorithm>

using std::copy;
using std::string;

const int Buffer::kInitialSize;

int Buffer::readableBytes() {
    return writer_index_ - reader_index_;
}

int Buffer::writableBytes() {
    return buffer_.size() - writer_index_;
}

int Buffer::discardReadBytes() {
    return reader_index_;
}

void Buffer::ensureWritableBytes(int len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }

    assert(writableBytes() >= len);
}

int Buffer::read(char* data, int len) {
    if (readableBytes() < len) {
        return -1;
    }

    copy(begin() + reader_index_, begin() + reader_index_ + len, data);
    reader_index_ += len;

    return len;
}

int Buffer::read(char* data) {
    int ret = read(data, readableBytes());
    if (-1 == ret) {
        return -1;
    }

    reader_index_ = 0;
    writer_index_ = 0;
    return 0;
}

int Buffer::read(void* data, int len) {
    return read(static_cast<char*>(data), len);
}

int Buffer::read(void *data) {
    return read(static_cast<char*>(data)); 
}

int Buffer::read(std::string& data, int len) {
    if (readableBytes() < len) {
        return -1;
    }

    data = string(begin() + reader_index_, len);
    reader_index_ += len;

    return len;
}

int Buffer::read(std::string& data) {
    int ret =  read(data, readableBytes());
    if (-1 == ret) {
        return -1;
    }

    reader_index_ = 0;
    writer_index_ = 0;

    return ret;
}

char* Buffer::peek() {
    return begin() + reader_index_;
}

int Buffer::retrieve(int len) {
    if (readableBytes() < len) {
        return -1;
    }

    reader_index_ += len;
    return len;
}

int Buffer::readInt64(int64_t& x) {
    int int64_t_size = static_cast<int>(sizeof(int64_t));
    if (readableBytes() < int64_t_size) {
        return -1;
    }

    int64_t n64 = 0;
    ::memcpy(&n64, begin() + reader_index_, int64_t_size);
    reader_index_ += int64_t_size;
    x = be64toh(n64);

    return int64_t_size;
}

int Buffer::readInt32(int32_t& x) {
    int int32_t_size = static_cast<int>(sizeof(int32_t));
    if (readableBytes() < int32_t_size) {
        return -1;
    }

    int32_t n32 = 0;
    ::memcpy(&n32, begin() + reader_index_, int32_t_size);
    reader_index_ += int32_t_size;
    x = be32toh(n32);

    return int32_t_size;
}

int Buffer::readInt16(int16_t& x) {
    int int16_t_size = static_cast<int>(sizeof(int16_t));
    if (readableBytes() < int16_t_size) {
        return -1;
    }

    int16_t n16 = 0;
    ::memcpy(&n16, begin() + reader_index_, sizeof(int16_t));
    reader_index_ += int16_t_size;
    x = be16toh(n16);

    return int16_t_size;
}

int Buffer::readInt8(int8_t& x) {
    int int8_t_size = static_cast<int>(sizeof(int8_t));
    if (readableBytes() < int8_t_size) {
        return -1;
    }

    int8_t n8 = 0;
    ::memcpy(&n8, begin() + reader_index_, int8_t_size);
    reader_index_ += int8_t_size;
    // FIXME network to host
    x = n8;

    return int8_t_size;
}

void Buffer::write(const char* data, int len) {
    ensureWritableBytes(len);
    copy(data, data + len, begin() + writer_index_);
    writer_index_ += len;
}

void Buffer::write(const void* data, int len) {
    write(static_cast<const char*>(data), len);
}

void Buffer::write(const std::string& data) {
    write(data.c_str(), data.size());
}

void Buffer::writeInt64(int64_t x) {
    int64_t n64 = htobe64(x);
    write(&n64, sizeof(n64));
}

void Buffer::writeInt32(int32_t x) {
    int32_t n32 = htobe32(x);
    write(&n32, sizeof(n32));
}

void Buffer::writeInt16(int16_t x) {
    int16_t n16 = htobe16(x);
    write(&n16, sizeof(n16));
}

void Buffer::writeInt8(int8_t x) {
    // FIXME host to network
    int8_t n8 = x;
    write(&n8, sizeof(n8));
}

// Implement with read(2)
int Buffer::readFd(int fd, int& _errno) {
    // saved an ioctl()/FIONREAD call to tell how much to read
    // 如果读入的数据不多，则全部读到buffer中；
    // 如果长度超过buffer的writable字节数目，就会读到栈上的extrabuf上
    char extrabuf[65536];
    struct iovec vec[2];
    int writable = writableBytes();

    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < static_cast<int>(sizeof(extrabuf))) ? 2 : 1;
    int n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        _errno = errno;
    } else if (n <= writable) {
        writer_index_ += n;
    } else {
        writer_index_ = buffer_.size();
        write(extrabuf, n - writable);
    }

    return n;
}

void Buffer::makeSpace(int len) {
    //
    // 如果buffer中可用的空间(可写字节数目+reader_index_之前的空间)>=len，则将readable
    // data向前移；
    // 否则，为buffer重新分配内存空间
    //
    if (reader_index_ + writableBytes() < len) {
        buffer_.resize(writer_index_ + len);
    } else {
        // move readable data to the front, make space inside buffer
        int readable = readableBytes();
        copy(begin() + reader_index_, begin() + writer_index_, begin());
        reader_index_ = 0;
        writer_index_ = readable;

        assert(readable == readableBytes());

    }
}
