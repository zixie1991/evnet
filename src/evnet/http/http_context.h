#ifndef EVNET_HTTP_HTTP_CONTEXT_H_
#define EVNET_HTTP_HTTP_CONTEXT_H_

class HttpContext {
  public:
    HttpContext();

  private:
    HttpRequest request_;
    HttpResponse response_;
};

#endif
