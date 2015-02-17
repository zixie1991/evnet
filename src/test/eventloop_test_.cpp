#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include <boost/bind.hpp>

#include "eventloop.h"
#include "log.h"

using boost::bind;

void print() {
    log_info("hello world, in queue");
}

int main() {
    EventLoop loop;
    loop.runInLoop(bind(&print));
    loop.queueInLoop(bind(&print));
    loop.loop();
}
