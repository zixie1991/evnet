#ifndef EVNET_HTTP_HTTP_REQUEST_H_
#define EVNET_HTTP_HTTP_REQUEST_H_

extern "C" {
#include "http_parser.h"
}

#include "../common.h"

class HttpRequest {
  public:
    enum Method{kDelete, kGet, kHead, kPost, kPut, kConnect, kOptions};

    HttpRequest();

    void set_remote_ip(const string& remote_ip) {
      remote_ip_ = remote_ip;
    }

    const string& remote_ip() const {
      return remote_ip_;
    }

    Method method() const {
      return method_;
    }

    void set_method(const Method& method) {
      method_ = method;
    }

    const char* MethodToString() const;

    string uri() {
      stringstream ss(url());
      string uri;
      std::getline(ss, uri, '?');
      return uri;
    }

    const string& url() const {
      return url_;
    }

    void set_url(const string& url) {
      url_ = url;
    }

    const string& version() const {
      return version_;
    }

    void set_version(const string& version) {
      version_ = version;
    }

    void add_header(const string& key, const string& val);
    string get_header(const string& key);

    void set_body(const string& body);
    void add_body(const string& str);
    const string& body() const;

    vector<string>& fields();
    vector<string>& values();

    void set_ok(bool ok);
    bool ok();

    bool Parse(const char* buff, int len);
    string GenRequest();

  private:
    http_parser_settings settings_;
    http_parser parser_;
    string remote_ip_;
    Method method_;
    string url_;
    string version_;
    map<string, string> headers_;
    string body_;

    vector<string> fields_;
    vector<string> values_;
    bool ok_;
};

#endif
