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
    EXPECT_EQ(buf.writableBytes(), 0);

    // write and read
    const string str(200, 'x');
    buf.write(str.c_str(), str.size());
    EXPECT_EQ(buf.readableBytes(), str.size());
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - str.size());

    const string str2 = buf.read(50);
    EXPECT_EQ(buf.readableBytes(), str.size() - str2.size());
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - str.size());

    buf.write(str.c_str(), str.size());
    EXPECT_EQ(buf.readableBytes(), 2 * str.size() - str2.size());
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize - 2 * str.size());
    
    const string str3 = buf.read(); 
    EXPECT_EQ(str3.size(), 350);
    EXPECT_EQ(buf.readableBytes(), 0);
    EXPECT_EQ(buf.writableBytes(), Buffer::kInitialSize);
}

TEST_F(BufferTest, testBufferGrow) {
    Buffer buf;

    string str = string(400, 'y');
    buf.write(str.c_str(), str.size());

}

TEST_F(BufferTest, testBufferInsideGrow) {

}

TEST_F(BufferTest, testBufferReadInt) {

}
