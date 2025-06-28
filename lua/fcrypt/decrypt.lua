-- lua/fcrypt/decrypt.lua

local logs = require("fcrypt.logs")
local buffer = require("core.buffer")
local fcryptc = require("fcrypt.native")

local M = {}

function M.decrypt_buf()
	local content = buffer.get_current_buffer_content()
	local password = vim.fn.inputsecret("Password: ")

	local ok_decrypt, result = fcryptc.decrypt_buf(content, password)
	if not ok_decrypt then
		logs.error("Decryption failed: " .. tostring(result))
		return
	end
	if result == nil then
		logs.error("Decrypted content is nil: " .. tostring(result))
		return
	end

	buffer.set_current_buffer_content(result)
end

return M
