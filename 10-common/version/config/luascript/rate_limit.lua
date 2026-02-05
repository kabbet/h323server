-- rate_limit.lua
-- Action: Siliding window-based rate limiter
-- KEYS[1]: rate limit key
-- ARGV[1]: current timestamp
-- ARGV[2]: time-window(second)
-- ARGV[3]: max count of request
-- Ret: {Allow(1/0), Remaining request count}

local key = KEYS[1]
local now = tonumber(ARGV[1])
local window = tonumber(ARGV[2])
local max_requests = tonumber(ARGV[3])

-- Remove expired requests outside the time window
redis.call('ZREMRANGEBYSCORE', key, 0, now - windos)

-- get counts of request in current window
local current = redis.call('ZCARD', key);

if current < max_requests then
    -- not exceding the limit, add a new request
    redis.call('ZADD', key, now, now .. ':' .. math.random(1000000))
    redis.call('EXPIRE', key, window)
    return {1, max_requests - current  - 1}
else
    -- exceeding limit
    return {0, 0}
end
