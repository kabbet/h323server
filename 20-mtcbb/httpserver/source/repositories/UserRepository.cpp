#include "repositories/UserRepository.hpp"
#include <drogon/orm/Mapper.h>
#include <drogon/orm/Criteria.h>
#include <trantor/utils/Logger.h>

using namespace repositories;
using namespace drogon::orm;
using namespace drogon_model::myapp;

void UserRepository::findUserById(
    const std::string& userId,
    UserCallback onSuccess,
    ErrorCallback onError)
{
    Mapper<Users> userMapper(dbClient_);

    userMapper.findBy(
        Criteria(Users::Cols::_user_id, CompareOperator::EQ, userId),
        [onSuccess](const std::vector<Users>& users) {  // 改为 const 引用
            if (users.empty()) {
                onSuccess(std::nullopt);
            } else {
                onSuccess(users[0]);
            }
        },
        [onError](const DrogonDbException& e) {  // 改为 DrogonDbException
            LOG_ERROR << "Database error: " << e.base().what();
            onError(e.base());
        }
    );
}

void UserRepository::saveToken(
    const UserTokens& token,
    std::function<void(bool)> onSuccess,
    ErrorCallback onError)
{
    Mapper<UserTokens> tokenMapper(dbClient_);

    tokenMapper.insert(
        token,
        [onSuccess](const UserTokens&) {  // 改为 const 引用
            onSuccess(true);
        },
        [onError](const DrogonDbException& e) {  // 改为 DrogonDbException
            LOG_ERROR << "Failed to save token: " << e.base().what();
            onError(e.base());
        }
    );
}

void UserRepository::findTokenByValue(
    const std::string& token,
    TokenCallback onSuccess,
    ErrorCallback onError)
{
    Mapper<UserTokens> tokenMapper(dbClient_);

    tokenMapper.findBy(
        Criteria(UserTokens::Cols::_token, CompareOperator::EQ, token),
        [onSuccess](const std::vector<UserTokens>& tokens) {  // 改为 const 引用
            if (tokens.empty()) {
                onSuccess(std::nullopt);
            } else {
                onSuccess(tokens[0]);
            }
        },
        [onError](const DrogonDbException& e) {  // 改为 DrogonDbException
            onError(e.base());
        }
    );
}

void UserRepository::deleteToken(
    const std::string& token,
    std::function<void(bool)> onSuccess,
    ErrorCallback onError)
{
    Mapper<UserTokens> tokenMapper(dbClient_);

    try {
        tokenMapper.deleteBy(
            Criteria(UserTokens::Cols::_token, CompareOperator::EQ, token)
        );
        onSuccess(true);
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to delete token: " << e.what();
        onError(e);
    }
}