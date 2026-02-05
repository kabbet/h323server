// include/LoginHandler.hpp
#ifndef LOGINHANDLER_HPP
#define LOGINHANDLER_HPP
#include <drogon/HttpResponse.h>
#include <functional>
#include <string>
#include <memory>
#include "models/Users.h"

namespace handler {

class LoginHandler : public std::enable_shared_from_this<LoginHandler> {
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    static void handle(const std::string& userId, 
                      const std::string& password,
                      ResponseCallback callback);

private:
    LoginHandler(const std::string& userId, 
                const std::string& password,
                ResponseCallback callback);

    void start();
    void onUserAuthenticated(const drogon_model::myapp::Users& user);
    void onAuthenticationFailed(const std::string& error, int statusCode);
    void onTokenCreated(const std::string& token);
    void onTokenCreationFailed(const std::string& error, int statusCode);
    void sendErrorResponse(const std::string& error, int statusCode);

    std::string userId_;
    std::string password_;
    ResponseCallback callback_;
    drogon_model::myapp::Users user_;  // 保存用户信息
};

// GetInfo Handler
class GetInfoHandler : public std::enable_shared_from_this<GetInfoHandler> {
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    static void handle(const std::string& userId,
                      const std::string& token,
                      ResponseCallback callback);

private:
    GetInfoHandler(const std::string& userId,
                  const std::string& token,
                  ResponseCallback callback);

    void start();
    void onTokenValidated();
    void onTokenValidationFailed(const std::string& error, int statusCode);
    void onUserInfoRetrieved(const drogon_model::myapp::Users& user);
    void onUserInfoFailed(const std::string& error, int statusCode);
    void sendErrorResponse(const std::string& error, int statusCode);

    std::string userId_;
    std::string token_;
    ResponseCallback callback_;
};

} // namespace handler
#endif

