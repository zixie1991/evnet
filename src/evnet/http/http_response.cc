#include "http_response.h"

const char* GetStatusDescription(int status_code) {
  switch (status_code) {
    case 200:
      return "OK"; // 客户端请求成功
    case 301:
      return "Moved Permanently"; // 请求永久重定向
    case 302:
      return "Moved Temporarily"; // 请求临时重定向
    case 304:
      return "Not Modified"; // 文件未修改，可以直接使用缓存的文件
    case 400:
      return "Bad Request"; // 由于客户端请求有语法错误，不能被服务器所理解
    case 403:
      return "Forbidden"; // 服务器拒绝提供服务
    case 404:
      return "Not Found"; // 请求资源不存在
    case 500:
      return "Internal Server Error"; // 服务器发生不可预期的错误，导致无法完成客户端的请求
    case 503:
      return "Service Unavailable"; // 务器当前不能够处理客户端的请求，在一段时间之后，服务器可能会恢复正常
  }
}

namespace {

int OnResponseStatus(http_parser* parser, const char* buf, size_t len) {
  HttpResponse* response = static_cast<HttpResponse*>(parser->data);
  response->set_status_code(parser->status_code);

  return 0;
}

int OnHeaderField(http_parser* parser, const char *buf, size_t len) {
  HttpResponse* response = static_cast<HttpResponse*>(parser->data);
  string field = string(buf, len);
  response->fields().push_back(field);

  if (response->version().empty()) {
    stringstream ss;
    ss << "HTTP/" << parser->http_major << "." << parser->http_minor;
    response->set_version(ss.str());
  }

  return 0;
}

int OnHeaderValue(http_parser* parser, const char* buf, size_t len) {
  HttpResponse* response = static_cast<HttpResponse*>(parser->data);
  string value = string(buf, len);
  response->values().push_back(value);
  response->add_header(response->fields()[response->fields().size() - 1], response->values()[response->values().size() - 1]);

  return 0;
}

int OnHeaderComplete(http_parser* parser) {
  HttpResponse* response = static_cast<HttpResponse*>(parser->data);

  return 0;
}

int OnBody(http_parser* parser, const char* buf, size_t len) {
  HttpResponse* response = static_cast<HttpResponse*>(parser->data);
  string body(buf, len);
  response->add_body(body);

  return 0;
}

int OnMessageComplete(http_parser* parser) {
  HttpResponse* response = static_cast<HttpResponse*>(parser->data);
  int is_final = http_body_is_final(parser);

  response->set_ok(is_final == 0);

  return 0;
}
}

HttpResponse::HttpResponse():
  ok_(false)
{
	http_parser_settings_init(&settings_);
	settings_.on_status = OnResponseStatus;
  settings_.on_header_field = OnHeaderField;
  settings_.on_header_value = OnHeaderValue;
  settings_.on_headers_complete = OnHeaderComplete;
  settings_.on_body = OnBody;
  settings_.on_message_complete = OnMessageComplete;

	http_parser_init(&parser_, HTTP_RESPONSE);
	parser_.data = this; 
}

void HttpResponse::set_version(const string& version) {
  version_ = version;
}

string HttpResponse::version() {
  return version_;
}

void HttpResponse::set_status_code(int status_code) {
  status_code_ = status_code;
}

int HttpResponse::status_code() {
  return status_code_;
}

void HttpResponse::add_header(const string& key, const string& val) {
  headers_[key] = val;
}

void HttpResponse::set_body(const string& body) {
  body_ = body;
  stringstream ss;
  ss << body.size();
  add_header("Content-Length", ss.str());
  ok_ = true;
}

void HttpResponse::add_body(const string& str) {
  body_.append(str);
}

vector<string>& HttpResponse::fields() {
  return fields_;
}

vector<string>& HttpResponse::values() {
  return values_;
}

void HttpResponse::set_ok(bool ok) {
  ok_ = ok;
}

bool HttpResponse::ok() {
  return ok_;
}

bool HttpResponse::Parse(const char* buf, size_t len) {
	ssize_t nparsed = http_parser_execute(&parser_, &settings_, buf, len);

  return nparsed == len;
}

string HttpResponse::GenResponse() {
  stringstream ss;
  ss << version_ << " " << status_code_ << " " << GetStatusDescription(status_code_) << "\r\n";
  for (auto iter = headers_.begin(); iter != headers_.end(); ++iter) {
    ss << iter->first << ":" << iter->second << "\r\n";
  }
  if (headers_.find("Content-Length") == headers_.end()) {
    ss << "Content-Length:" << body_.size() << "\r\n";
  }
  ss << "\r\n";
  ss << body_;

  return ss.str();
}
