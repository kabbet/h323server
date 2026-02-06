-- batch_set_hash.lua
-- Action: batch set multiple field in hash, optionally setting an expiration
-- time
-- KEYS[1]: key of hash
-- ARGV[1]: expire time (second, 0-not set)
-- ARGV[2...N]: field, value1, field2, value2, ...
-- Ret: amount of setting values

-- lua/batch_set_hash.lua
redis.log(redis.LOG_WARNING, "=== batch_set_hash DEBUG START ===")
redis.log(redis.LOG_WARNING, "KEYS count: " .. #KEYS)
for i = 1, #KEYS do
    redis.log(redis.LOG_WARNING, "KEYS[" .. i .. "]: " .. tostring(KEYS[i]))
end

redis.log(redis.LOG_WARNING, "ARGV count: " .. #ARGV)
for i = 1, #ARGV do
    redis.log(redis.LOG_WARNING, "ARGV[" .. i .. "]: " .. tostring(ARGV[i]))
end

local key = KEYS[1]

-- ✅ 修复：清理字符串，去除引号和空格
local expire_str = ARGV[1]
expire_str = string.gsub(expire_str, '"', '')  -- 移除双引号
expire_str = string.gsub(expire_str, "'", '')  -- 移除单引号
expire_str = string.gsub(expire_str, "^%s*(.-)%s*$", "%1")  -- 移除首尾空格

redis.log(redis.LOG_WARNING, "expire_str after cleanup: [" .. expire_str .. "]")

local expire = tonumber(expire_str)
local count = 0

redis.log(redis.LOG_WARNING, "expire after tonumber: " .. tostring(expire))

if not expire then
    redis.log(redis.LOG_WARNING, "ERROR: expire is nil after cleanup!")
    redis.log(redis.LOG_WARNING, "Original ARGV[1]: [" .. tostring(ARGV[1]) .. "]")
    redis.log(redis.LOG_WARNING, "Cleaned expire_str: [" .. expire_str .. "]")
    redis.log(redis.LOG_WARNING, "ARGV[1] length: " .. string.len(ARGV[1]))
    redis.log(redis.LOG_WARNING, "ARGV[1] byte values: " .. string.byte(ARGV[1], 1, -1))
    return redis.error_reply("Invalid expire time, ARGV[1]=" .. tostring(ARGV[1]))
end

-- batch set field
redis.log(redis.LOG_WARNING, "Starting HSET operations...")
for i = 2, #ARGV, 2 do
    if ARGV[i] and ARGV[i + 1] then
        redis.log(redis.LOG_WARNING, "HSET " .. key .. " " .. ARGV[i] .. " = " .. ARGV[i+1])
        redis.call('HSET', key, ARGV[i], ARGV[i+1])
        count = count + 1
    else
        redis.log(redis.LOG_WARNING, "Skipping pair at index " .. i .. " (nil value)")
    end
end

if expire > 0 then
    redis.log(redis.LOG_WARNING, "Setting EXPIRE: " .. expire .. " seconds")
    redis.call('EXPIRE', key, expire)
else
    redis.log(redis.LOG_WARNING, "No expiration set (expire <= 0)")
end

redis.log(redis.LOG_WARNING, "Total fields set: " .. count)
redis.log(redis.LOG_WARNING, "=== batch_set_hash DEBUG END ===")

return count
