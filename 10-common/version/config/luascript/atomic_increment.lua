-- atomic_increment.lua
-- Action: Actomically increment the counter and if it exceeds the maximum value
-- KEYS[1]: key of counter
-- ARGV[1]: Amount of increment
-- ARGV[2]: Maximum(optional, 0-unlimit)
-- ARGV[3]: expire time (second, optional, 0-unset)
-- Ret: {success(1/0), new value}

local key = KEYS[1]
local increment = tonumber(ARGV[1])
local max_value = tonumber(ARGV[2])
local expire = tonumber(ARGV[3])

-- get current value
local current = tonumber(redis.call('GET', key)) or 0
local new_value = current + increment

-- check whether exceeds the maximum limit
if max_value > 0 and new_value > max_value then
    return {0, current}
end

-- increment and set new value
redis.call('SET', key, new_value)

-- if specify expire time, to set
if expire > 0 then
    redis.call('EXPIRE', key, expire)
end

return {1, new_value}
