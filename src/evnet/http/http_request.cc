#include "http_request.h"

namespace {

int OnRequestUrl(http_parser* parser, const char* buf, size_t len) {
  HttpRequest* request = static_cast<HttpRequest*>(parser->data);
  string url(buf, len);
  request->set_url(url);
  request->set_method(HttpRequest::Method(parser->method));

  return 0;
}

int OnHeaderField(http_parser* parser, const char *buf, size_t len) {
  HttpRequest* request = static_cast<HttpRequest*>(parser->data);
  string field = string(buf, len);
  request->fields().push_back(field);

  if (request->version().empty()) {
    stringstream ss;
    ss << "HTTP/" << parser->http_major << "." << parser->http_minor;
    request->set_version(ss.str());
  }

  return 0;
}

int OnHeaderValue(http_parser* parser, const char* buf, size_t len) {
  HttpRequest* request = static_cast<HttpRequest*>(parser->data);
  string value = string(buf, len);
  request->values().push_back(value);
  request->add_header(request->fields()[request->fields().size() - 1], request->values()[request->values().size() - 1]);

  return 0;
}

int OnHeaderComplete(http_parser* parser) {
  HttpRequest* request = static_cast<HttpRequest*>(parser->data);

  return 0;
}

int OnBody(http_parser* parser, const char* buf, size_t len) {
  HttpRequest* request = static_cast<HttpRequest*>(parser->data);
  string body(buf, len);
  request->add_body(body);

  return 0;
}

int OnMessageComplete(http_parser* parser) {
  HttpRequest* request = static_cast<HttpRequest*>(parser->data);
  int is_final = http_body_is_final(parser);

  request->set_ok(is_final == 0);

  return 0;
}
}

HttpRequest::HttpRequest():
  ok_(false)
{
	http_parser_settings_init(&settings_);
	settings_.on_url = OnRequestUrl;
  settings_.on_header_field = OnHeaderField;
  settings_.on_header_value = OnHeaderValue;
  settings_.on_headers_complete = OnHeaderComplete;
  settings_.on_body = OnBody;
  settings_.on_message_complete = OnMessageComplete;

	http_parser_init(&parser_, HTTP_REQUEST);
	parser_.data = this; 
}

const char* HttpRequest::MethodToString() const {
  switch (method_) {
    case kDelete:
      return "DELETE";
    case kGet:
      return "GET";
    case kHead:
      return "HEAD";
    case kPost:
      return "POST";
    case kPut:
      return "PUT";
    case kConnect:
      return "CONNECT";
    case kOptions:
      return "OPTIONS";
    default:
      return "unsupport method";
  }
}

void HttpRequest::add_header(const string& key, const string& val) {
  headers_[key] = val;
}

string HttpRequest::get_header(const string& key) {
  auto iter = headers_.find(key);
  if (iter != headers_.end()) {
    return iter->second;
  }

  return "";
}

void HttpRequest::set_body(const string& body) {
  body_ = body;
  stringstream ss;
  ss << body.size();
  add_header("Content-Length", ss.str());
  ok_ = true;
}

void HttpRequest::add_body(const string& str) {
  body_.append(str);
}

const string& HttpRequest::body() const {
  return body_;
}

vector<string>& HttpRequest::fields() {
  return fields_;
}

vector<string>& HttpRequest::values() {
  return values_;
}

void HttpRequest::set_ok(bool ok) {
  ok_ = ok;
}

bool HttpRequest::ok() {
  return ok_;
}

bool HttpRequest::Parse(const char* buf, int len) {
	ssize_t nparsed = http_parser_execute(&parser_, &settings_, buf, len);

  return nparsed == len;
}

string HttpRequest::GenRequest() {
  stringstream ss;
  ss << MethodToString() << " " << url_ << " " << version_ << "\r\n";
  for (auto iter = headers_.begin(); iter != headers_.end(); ++iter) {
    ss << iter->first << ":" << iter->second << "\r\n";
  }
  ss << "\r\n";
  ss << body_;

  return ss.str();
}
