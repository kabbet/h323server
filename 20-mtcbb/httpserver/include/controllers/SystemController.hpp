#ifndef SYSTEMCONTROLLER_HPP
#define SYSTEMCONTROLLER_HPP

#include <drogon/HttpController.h>
#include "interfaces/ISystemService.hpp"
#include "infrastructure/ServiceContainer.hpp"
#include <memory>

using namespace drogon;

namespace api {
namespace v1 {
namespace system {

class System : public HttpController<System> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(System::getAccountToken, "/api/v1/system/token",   Post);
    ADD_METHOD_TO(System::login,           "/api/v1/system/login",   Post);
    ADD_METHOD_TO(System::getVersion,      "/api/v1/system/version", Get);
    METHOD_LIST_END

    // ✅ 无参构造，什么都不做，不碰 ServiceContainer
    System() = default;

    void getAccountToken(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void login(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);

    void getVersion(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback) const;

private:
    /**
     * ✅ 懒加载：每次请求时才从 ServiceContainer 取，
     *    此时 registerBeginningAdvice 已执行完，服务必然已初始化。
     */
    std::shared_ptr<interfaces::ISystemService> getService() const {
        return ServiceContainer::instance().getSystemService();
    }

    static HttpResponsePtr makeSuccess(Json::Value extra = Json::objectValue);
    static HttpResponsePtr makeError(int errorCode, const std::string& msg,
                                     HttpStatusCode httpStatus = k200OK);
};

} // namespace system
} // namespace v1
} // namespace api

#endif // SYSTEMCONTROLLER_HPP
