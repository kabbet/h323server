-- acquire_lock.lua
-- Action: Get a Distrubuted lock
-- KEYS[1]: lock key
-- ARGV[1]: key value(unique tag)
-- ARGV[2]: expire time(second)
-- Ret: 1-success, 0-failed

local key = KEYS[1]
local value = ARGV[1]
local expire = tonumber(ARGV[2])

-- use nx option of set command (set only if the key does not exist)
local result = redis.call('SET', key, value, 'NX', 'EX', expire)

if result then
    return 1
else
    return 0
end
