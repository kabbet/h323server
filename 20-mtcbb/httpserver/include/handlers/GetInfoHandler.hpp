// include/handlers/LoginHandler.hpp
#ifndef GETINFOHANDLER_HPP
#define GETINFOHANDLER_HPP

#include <drogon/HttpResponse.h>
#include <functional>
#include <string>
#include <memory>
#include "models/Users.h"
#include "interfaces/IUserService.hpp"
#include "services/TokenCleanupService.hpp"

namespace handlers {
class GetInfoHandler : public std::enable_shared_from_this<GetInfoHandler> {
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    explicit GetInfoHandler(std::shared_ptr<interfaces::IUserService> userService)
        : userService_(userService) {}

    static void handle(
        std::shared_ptr<interfaces::IUserService> userService,
        const std::string& userId,
        const std::string& token,
        ResponseCallback callback);

private:
    GetInfoHandler(
        std::shared_ptr<interfaces::IUserService> userService,
        const std::string& userId,
        const std::string& token,
        ResponseCallback callback);

    void start();
    void onTokenValidated();
    void onTokenValidationFailed(const std::string& error, int statusCode);
    void onUserInfoRetrieved(const drogon_model::myapp::Users& user);
    void onUserInfoFailed(const std::string& error, int statusCode);
    void sendErrorResponse(const std::string& error, int statusCode);

    std::shared_ptr<interfaces::IUserService> userService_;  // 注入的依赖
    std::string userId_;
    std::string token_;
    ResponseCallback callback_;
};

} // namespace handlers

#endif
