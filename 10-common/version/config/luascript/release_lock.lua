-- release_lock.lua
-- Action: release distributed lock
-- KEYS[1]: lock key
-- ARGV[1]: key value (unique tag)
-- Ret: 1-success, 0-failed(lock not exists or is not owner by the current
-- client)

local key = KEYS[1]
local value = ARGV[1]

-- check whether the lock exists and whether the value matches
if redis.call('GET', key) == value then
    -- own by the current client, delete it
    redis.call('DEL', key)
    return 1
else
    -- is not owner by the current client or lock is not existing
    return 0
end

