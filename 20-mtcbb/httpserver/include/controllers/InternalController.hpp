#ifndef INTERNALCONTROLLER_HPP
#define INTERNALCONTROLLER_HPP

#include "infrastructure/ServiceContainer.hpp"
#include "interfaces/ISystemService.hpp"
#include <drogon/HttpController.h>
#include <functional>

namespace drogon {
/**
 * 内部接口 Controller
 * 仅供 Jetty 网关调用，不对外暴露
 *
 * 路由
 *  GET /internal/session/validate <- 网关握手鉴权
 *  POST /internal/event/publish   <- C++业务层触发推送
 */
class Internal : public HttpController<Internal>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(Internal::validateSession, "/internal/session/validate", Get);
    ADD_METHOD_TO(Internal::publishEvent, "/internal/event/publish", Post);
    METHOD_LIST_END

    Internal() = default;

    // GET /internal/session/validate
    // Params: account_token=xxx&sso_cookie=xxx
    // 供 Jetty 网关握手时调用
    void validateSession(const HttpRequestPtr&                         req,
                         std::function<void(const HttpResponsePtr&)>&& callback);

    // POST /internal/event/publish
    // Body: {"channel": "/userdomains/.../confs/.../mts/1", "method":
    // "update"}
    // C++业务层产生事件后调用此接口，转发给Jetty网关推送给终端
    void publishEvent(const HttpRequestPtr&                         req,
                      std::function<void(const HttpResponsePtr&)>&& callback);

private:
    std::shared_ptr<interfaces::ISystemService> getService() const
    {
        return ServiceContainer::instance().getSystemService();
    }

    static const std::string kGatewayPublishUrl;

    static HttpResponsePtr makeSuccess(Json::Value extra = Json::objectValue);
    static HttpResponsePtr makeError(int errorCode, const std::string& msg,
                                     HttpStatusCode status = k200OK);
};
}   // namespace drogon

#endif
