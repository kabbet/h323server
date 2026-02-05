-- get_next_id.lua
-- Action: Atomically generate a monotonically increasing unique ID
-- KEYS[1]: counter's key
-- Ret: newId

local key = KEYS[1]
local id = redis.call('INCR', key)
return id
