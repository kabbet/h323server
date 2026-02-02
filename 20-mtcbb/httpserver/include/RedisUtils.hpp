#ifndef REDISUTILS_HPP
#define REDISUTILS_HPP

#include "drogon/drogon.h"
#include <drogon/nosql/RedisClient.h>
#include <drogon/nosql/RedisResult.h>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <trantor/utils/Logger.h>

using namespace drogon;
using namespace drogon::nosql;

class RedisUtils
{
public:
    static RedisUtils& instance()
    {
        static RedisUtils inst;
        return inst;
    }

    // strings operator
    // set key value
    void set(const std::string& key, const std::string& value,
             const std::function<void(bool)> callback, int expireSeconds = 0)
    {
        if (expireSeconds > 0) {
            client_->execCommandAsync(
                [callback](const RedisResult& r) { callback(r.asString() == "OK"); },
                [callback](const std::exception& e) {
                    LOG_ERROR << "Redis SET error: " << e.what();
                    callback(false);
                },
                "SET %s %d %s",
                key.c_str(),
                expireSeconds,
                value.c_str());
        }
        else {
            client_->execCommandAsync(
                [callback](const RedisResult& r) { callback(r.asString() == "OK"); },
                [callback](const std::exception& e) {
                    LOG_ERROR << "Redis SET error: " << e.what();
                    callback(false);
                },
                "SET %s %s",
                key.c_str(),
                value.c_str());
        }
    }

    // GET key
    void get(const std::string& key, std::function<void(std::optional<std::string>)> callback)
    {
        client_->execCommandAsync(
            [callback](const RedisResult& r) {
                if (r.type() == RedisResultType::kNil) {
                    callback(std::nullopt);
                }
                else {
                    callback(r.asString());
                }
            },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis Get Error: " << e.what();
                callback(std::nullopt);
            },
            "GET %s",
            key.c_str());
    }

    // DEL key
    void del(const std::string& key, std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger() > 0); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis DEL error: " << e.what();
                                      callback(false);
                                  },
                                  "DEL %s",
                                  key.c_str());
    }

    // EXISTS key
    void exists(const std::string& key, std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger() > 0); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis EXISTS error: " << e.what();
                                      callback(false);
                                  },
                                  "EXISTS %s",
                                  key.c_str());
    }

    // EXPIRE key seconds
    void expire(const std::string& key, int seconds, std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger() > 0); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis EXPIRE error: " << e.what();
                                      callback(false);
                                  },
                                  "EXPIRE %s %d",
                                  key.c_str(),
                                  seconds);
    }

    // TTL key
    void ttl(const std::string& key, std::function<void(int)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger()); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis TTL error: " << e.what();
                                      callback(-2);
                                  },
                                  "TTL %s",
                                  key.c_str()

        );
    }

    // Has operator

    // HSET key field value
    void hset(const std::string& key, const std::string& field, const std::string& value,
              std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(true); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis HSET error: " << e.what();
                                      callback(false);
                                  },
                                  "HSET %s %s %s",
                                  key.c_str(),
                                  field.c_str(),
                                  value.c_str());
    }


    // HGet key field
    void hget(const std::string& key, const std::string& field,
              std::function<void(std::optional<std::string>)> callback)
    {
        client_->execCommandAsync(
            [callback](const RedisResult& r) {
                if (r.type() == RedisResultType::kNil) {
                    callback(std::nullopt);
                }
                else {
                    callback(r.asString());
                }
            },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis HGET error: " << e.what();
                callback(std::nullopt);
            },
            "HGET %s %s",
            key.c_str(),
            field.c_str());
    }

    // HGETALL key
    void hgetall(const std::string&                                      key,
                 std::function<void(std::map<std::string, std::string>)> callback)
    {
        client_->execCommandAsync(
            [callback](const RedisResult& r) {
                std::map<std::string, std::string> result;
                if (r.type() == RedisResultType::kArray) {
                    auto arr = r.asArray();
                    for (size_t i = 0; i < arr.size(); i += 2) {
                        if (i + 1 < arr.size()) {
                            result[arr[i].asString()] = arr[i + 1].asString();
                        }
                    }
                }
                callback(result);
            },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis HGETALL error: " << e.what();
                callback({});
            },
            "HGETALL %s",
            key.c_str());
    }

    // HDEL key field
    void hdel(const std::string& key, const std::string& field, std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger() > 0); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis HDEL error: " << e.what();
                                      callback(false);
                                  },
                                  "HDEL %s %s",
                                  key.c_str(),
                                  field.c_str());
    }

    // List operator

    // LPUSH key value
    void lpush(const std::string& key, const std::string& value, std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(true); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis LPUSH error: " << e.what();
                                      callback(false);
                                  },
                                  "LPUSH %s %s",
                                  key.c_str(),
                                  value.c_str());
    }

    // RPUSH key value
    void rpush(const std::string& key, const std::string& value, std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(true); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis RPUSH error: " << e.what();
                                      callback(false);
                                  },
                                  "RPUSH %s %s",
                                  key.c_str(),
                                  value.c_str());
    }

    // LRANGE key start stop
    void lrange(const std::string& key, int start, int stop,
                std::function<void(std::vector<std::string>)> callback)
    {
        client_->execCommandAsync(
            [callback](const RedisResult& r) {
                std::vector<std::string> result;
                if (r.type() == RedisResultType::kArray) {
                    auto arr = r.asArray();
                    for (const auto& item : arr) {
                        result.push_back(item.asString());
                    }
                }
                callback(result);
            },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis LRANGE error: " << e.what();
                callback({});
            },
            "LRANGE %s %d %d",
            key.c_str(),
            start,
            stop);
    }

    // SET OPEARTOR

    // SADD key member
    void sadd(const std::string& key, const std::string& member, std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger() > 0); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis SADD error: " << e.what();
                                      callback(false);
                                  },
                                  "SADD %s %s",
                                  key.c_str(),
                                  member.c_str());
    }

    // SMEMBERS key
    void smembers(const std::string& key, std::function<void(std::vector<std::string>)> callback)
    {
        client_->execCommandAsync(
            [callback](const RedisResult& r) {
                std::vector<std::string> result;
                if (r.type() == RedisResultType::kArray) {
                    auto arr = r.asArray();
                    for (const auto& item : arr) {
                        result.push_back(item.asString());
                    }
                }
                callback(result);
            },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis SMEMBERS error: " << e.what();
                callback({});
            },
            "SMEMBERS %s",
            key.c_str());
    }

    // SISMEMBER key member
    void sismember(const std::string& key, const std::string& member,
                   std::function<void(bool)> callback)
    {
        client_->execCommandAsync([callback](const RedisResult& r) { callback(r.asInteger() > 0); },
                                  [callback](const std::exception& e) {
                                      LOG_ERROR << "Redis SISMEMBER error: " << e.what();
                                      callback(false);
                                  },
                                  "SISMEMBER %s %s",
                                  key.c_str(),
                                  member.c_str());
    }

private:
    RedisUtils() { client_ = app().getRedisClient(); }
    std::shared_ptr<RedisClient> client_;
};

#endif
