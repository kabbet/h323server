-- validate_token.lua
-- Action: Verify whether the token exists; if it does, return the user ID and
-- refresh the expiration time
-- KEYS[1]: token
-- ARGV[1]: new expiration time
-- Ret: userId or nil

local token = KEYS[1]
local expire_time = tonumber(ARGV[1])

local userId = redis.call('GET', token)
if userId then
    -- token exists, refresh the expiration time
    redis.call('EXPIRE', token, expire_time)
    return userId
else
    -- token not exists or expired
    return nil
end
