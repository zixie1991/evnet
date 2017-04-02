#include <gtest/gtest.h>

#include "http_response.h"

TEST(HttpResponseTest, TestParse) {
#define RESPONSE "HTTP/1.1 200 OK\r\n"\
         "Server:Apache Tomcat/5.0.12\r\n" \
         "Date:Mon,6Oct2003 13:23:42 GMT\r\n"\
         "Content-Length:13\r\n"\
         "\r\n"\
         "<html></html>"
  HttpResponse response;
  response.Parse(RESPONSE, strlen(RESPONSE));

  if (response.ok()) {
    LOG(INFO) << response.GenResponse();
  }
}
