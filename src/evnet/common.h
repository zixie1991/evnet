#ifndef EVNET_COMMON_H_
#define EVNET_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <iostream>

#include <glog/logging.h>
#include <boost/any.hpp>

using std::string;
using std::vector;
using std::queue;
using std::priority_queue;
using std::set;
using std::map;
using std::unique_ptr;
using std::shared_ptr;
using std::enable_shared_from_this;
using std::function;
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::thread;
using std::mutex;
using std::lock_guard;
using std::unique_lock;
using std::condition_variable;
using std::stringstream;

using boost::any;
using boost::any_cast;

#endif
