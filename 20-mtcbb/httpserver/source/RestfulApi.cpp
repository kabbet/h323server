#include "RestfulApi.hpp"
#include <drogon/HttpSimpleController.h>
#include <drogon/HttpResponse.h>

namespace drogon
{
    void SayHello::genericHello(const HttpRequestPtr & tmp, std::function<void(const HttpResponsePtr&)>&& callback)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(
                "hello, this is a generic hello message from the SayHello"
                "controller"
                );
        callback(resp);
    }

    void SayHello::personalizedHello(const HttpRequestPtr& tmp, std::function<void(const HttpResponsePtr&)> && callback)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(
                "Hi there, this is another hello from the SayHello Controller"
                );
        callback(resp);
    }

    void HelloViewController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)> &&callback) 
    {
        HttpViewData data;
        data["name"] = req->getParameter("name");
        auto resp = HttpResponse::newHttpViewResponse("HelloView", data);
        callback(resp);
    }
}
