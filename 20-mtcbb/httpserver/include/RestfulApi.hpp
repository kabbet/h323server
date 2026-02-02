#ifndef RESTFULAPI_HPP
#define RESTFULAPI_HPP

#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpSimpleController.h>

using namespace drogon;

namespace api {
namespace v1 {
class User : public HttpController<User>
{
public:
    METHOD_LIST_BEGIN
    METHOD_ADD(User::login, "/token", Post);
    METHOD_ADD(User::getInfo, "/{1}info?token={2}", Get);
    METHOD_LIST_END

public:
    void login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getInfo(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback,
                 std::string userId, const std::string token) const;
};
}   // namespace v1
}   // namespace api

#endif
