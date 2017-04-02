#ifndef EVNET_HTTP_HTTP_RESPONSE_H_
#define EVNET_HTTP_HTTP_RESPONSE_H_

extern "C" {
#include "http_parser.h"
}

#include "../common.h"

const char* GetStatusDescription(int status_code);

class HttpResponse {
  public:
    HttpResponse();

    void set_version(const string& version);
    string version();
    void set_status_code(int status_code);
    int status_code();

    void add_header(const string& key, const string& val);
    void set_body(const string& body);
    void add_body(const string& str);

    vector<string>& fields();
    vector<string>& values();

    void set_ok(bool ok);
    bool ok();

    bool Parse(const char* buf, size_t len);
    string GenResponse();

  private:
    http_parser_settings settings_;
    http_parser parser_;
    string version_;
    int status_code_;
    vector<string> fields_;
    vector<string> values_;
    map<string, string> headers_;
    string body_;
    bool ok_;
};

#endif
