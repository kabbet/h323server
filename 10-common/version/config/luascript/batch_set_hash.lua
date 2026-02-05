-- batch_set_hash.lua
-- Action: batch set multiple field in hash, optionally setting an expiration
-- time
-- KEYS[1]: key of hash
-- ARGV[1]: expire time (second, 0-not set)
-- ARGV[2...N]: field, value1, field2, value2, ...
-- Ret: amount of setting values

local key = KEYS[1]
local expire = tonumber(ARGV[1])
local count = 0

-- batch set field
for i = 2, #ARGV, 2 do
    if ARGV[i] and ARGV[i + 1] then
        redis.call('HSET', key, ARGV[i], ARGV[i+1])
        count = count + 1
    end
end

-- if specify expire time, to set
if expire > 0 then
    redis.call('EXPIRE', key, expire)
end

return count
