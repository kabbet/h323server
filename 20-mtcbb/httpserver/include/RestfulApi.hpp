#ifndef RESTFULAPI_HPP
#define RESTFULAPI_HPP

#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpSimpleController.h>

namespace drogon {
class SayHello : public HttpController<SayHello>
{
public:
    METHOD_LIST_BEGIN
    METHOD_ADD(SayHello::genericHello, "/", Get);
    METHOD_ADD(SayHello::personalizedHello, "/hello", Get);
    METHOD_LIST_END
protected:
    void genericHello(const HttpRequestPtr&,
                      std::function<void(const HttpResponsePtr&)>&& callback);
    void personalizedHello(const HttpRequestPtr&,
                           std::function<void(const HttpResponsePtr&)>&& callback);
};

class HelloViewController : public HttpSimpleController<HelloViewController>
{
public:
    PATH_LIST_BEGIN
    PATH_ADD("/view");
    PATH_LIST_END
    void asyncHandleHttpRequest(const HttpRequestPtr&                         req,
                                std::function<void(const HttpResponsePtr&)>&& callback) override;
};
}   // namespace drogon


#endif
