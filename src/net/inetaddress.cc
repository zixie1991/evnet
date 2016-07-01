#include "inetaddress.h"

#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IPV4ANYADDR "0.0.0"

using std::string;

InetAddress::InetAddress(uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);

  if (::inet_pton(AF_INET, IPV4ANYADDR, &addr_.sin_addr) != 1) {
    // convert ipv4 from text to binary form error
  }
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);

  if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) != 1) {
      // convert ipv4 from text to binary form error
  }
}

std::string InetAddress::ip() const {
  char buf[32];
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof(addr_))); 

  return buf;
}

uint16_t InetAddress::port() const {
  return htobe16(addr_.sin_port); 
}
