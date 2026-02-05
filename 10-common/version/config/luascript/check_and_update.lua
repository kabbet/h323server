-- check_and_update.lua
-- Action: check whether the value meets the condition, and update it if it does
-- KEYS[1]: key
-- ARGV[1]: excepted minimum value
-- ARGV[2]: Amount of decrease
-- Ret: {Success flag: (1/0), current value or new value}

local key = KEYS[1]
local min_value = tonumber(ARGV[1])
local decrement = tonumber(ARGV[2])

-- get current value
local current = tonumber(redis.call('GET', key))

if not current then 
    -- key is not exist
    return {0, -1}
end

-- check whether the value meets the minimu condition
if current < min_value + decrement then
    -- left could not meet conditoin
    return {0, current}
end

-- Decrement and return the new value
local new_value = current - decrement
redis.call('SET', key, new_value)

return {1, new_value}
