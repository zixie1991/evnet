#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include "buffer.h"

using namespace std;

class BufferTest: public ::testing::Test {
    protected:
        virtual void SetUp() {
        }

        virtual void TearDown() {
        }
        
};


TEST_F(BufferTest, testBufferWriteAndRead) {
    Buffer buf;
    // Buffer构造函数测试
    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize);

    // write and read
    const string str(200, 'x');
    buf.write(str.c_str(), str.size());
    EXPECT_EQ(buf.readableBytes(), str.size());
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - str.size());

    string str2;
    buf.read(str2, 50);
    EXPECT_EQ(buf.readableBytes(), str.size() - str2.size());
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - str.size());

    buf.write(str.c_str(), str.size());
    EXPECT_EQ(buf.readableBytes(), 2 * str.size() - str2.size());
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - 2 * str.size());
    
    string str3;
    buf.read(str3); 
    EXPECT_EQ(str3.size(), 350);
    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize);
}

TEST_F(BufferTest, testBufferGrow) {
    Buffer buf;

    buf.write(string(400, 'y'));
    EXPECT_EQ(buf.readableBytes(), 400);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - 400);

    string str;
    buf.read(str, 50);
    EXPECT_EQ(buf.readableBytes(), 350);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - 400);

    buf.write(string(1000, 'z'));
    EXPECT_EQ(buf.readableBytes(), 1350);
    EXPECT_EQ(buf.writableBytes(), 0);

    buf.read(str);
    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writableBytes(), 1350);
}

TEST_F(BufferTest, testBufferInsideGrow) {
    Buffer buf;
    
    buf.write(string(800, 'y'));
    EXPECT_EQ(buf.readableBytes(), 800);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - 800);

    string str;
    buf.read(str, 500);
    EXPECT_EQ(buf.readableBytes(), 300);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - 800);

    buf.write(string(300, 'z'));
    EXPECT_EQ(buf.readableBytes(), 600);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - 600);
}

TEST_F(BufferTest, testBufferReadInt) {
    Buffer buf;

    buf.write("HTTP1.1");
    EXPECT_EQ(buf.readableBytes(), 7);
    int16_t x16 = 0;
    buf.readInt16(x16);
    EXPECT_EQ(x16, 'T' * 256 + 'H');
    int8_t x8 = 0;
    buf.readInt8(x8);
    EXPECT_EQ(x8, 'T');
    int32_t x32 = 0;
    buf.readInt32(x32);
    EXPECT_EQ(x32, '1' * 16777216 + '.' * 65536 + '1' * 256 + 'P');

    buf.writeInt8(-1); // 1 byte
    buf.writeInt16(-1); // 2 bytes
    buf.writeInt32(-1); // 4 bytes
    EXPECT_EQ(buf.readableBytes(), 7);
    int8_t h8 = 0;
    buf.readInt8(h8);
    EXPECT_EQ(h8, -1);
    int16_t h16 = 0;
    buf.readInt16(h16);
    EXPECT_EQ(h16, -1);
    int32_t h32 = 0;
    buf.readInt32(h32);
    EXPECT_EQ(h32, -1);
}
