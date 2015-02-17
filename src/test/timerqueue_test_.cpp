#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include <boost/bind.hpp>

#include "eventloop.h"
#include "ptime.h"
#include "log.h"

using boost::bind;

void timer_callback(const Timestamp& when) {
    log_info("timer callback run when %s", when.toString().c_str()); 
}

int main() {
    set_log_level(Logger::LEVEL_DEBUG);
    EventLoop loop;
    Timestamp time;
    time.now();
    time.set_microseconds(time.microseconds() + 1000000);

    loop.runAt(time, bind(&timer_callback, time));
    loop.runAfter(time, 0.1, bind(&timer_callback, time));
    loop.runRepeat(time, 2, bind(&timer_callback, time));

    loop.loop();
    return 0;
}
