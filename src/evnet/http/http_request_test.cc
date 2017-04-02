#include <gtest/gtest.h>

#include "http_request.h"

TEST(HttpRequestTest, TestParse) {
#define UPGRADE_POST_REQUEST "POST /demo HTTP/1.1\r\n"\
         "Host: example.com\r\n"\
         "Connection: Upgrade\r\n"\
         "Upgrade: HTTP/2.0\r\n"\
         "Content-Length: 15\r\n"\
         "\r\n"\
         "sweet post body"\
         "Hot diggity dogg"
  HttpRequest request;
  //request.Parse(UPGRADE_POST_REQUEST, strlen(UPGRADE_POST_REQUEST));
  request.Parse(UPGRADE_POST_REQUEST, 20);
  request.Parse(UPGRADE_POST_REQUEST + 20, strlen(UPGRADE_POST_REQUEST) - 20);

  if (request.ok()) {
    LOG(INFO) << request.GenRequest();
  }
}
