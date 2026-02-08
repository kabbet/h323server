#ifndef USERREPOSITORY_HPP
#define USERREPOSITORY_HPP

#include "interfaces/IUserRepository.hpp"
#include <drogon/orm/DbClient.h>

namespace repositories {

class UserRepository : public interfaces::IUserRepository {
public:
    explicit UserRepository(drogon::orm::DbClientPtr dbClient)
        : dbClient_(dbClient) {}

    void findUserById(
        const std::string& userId,
        UserCallback onSuccess,
        ErrorCallback onError) override;

    void saveToken(
        const drogon_model::myapp::UserTokens& token,
        std::function<void(bool)> onSuccess,
        ErrorCallback onError) override;

    void findTokenByValue(
        const std::string& token,
        TokenCallback onSuccess,
        ErrorCallback onError) override;

    void deleteToken(
        const std::string& token,
        std::function<void(bool)> onSuccess,
        ErrorCallback onError) override;

private:
    drogon::orm::DbClientPtr dbClient_;
};

} // namespace repositories

#endif
