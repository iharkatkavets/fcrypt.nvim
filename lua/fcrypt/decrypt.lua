-- lua/fcrypt/decrypt.lua

local logs = require("fcrypt.logs")
local buffer = require("core.buffer")
local fcryptc = require("fcrypt.native")

local M = {}

function M.decrypt_buf()
	local ok_content, buf_content = buffer.get_current_buffer_file_content()
	if not ok_content then
		logs.error("\nCan't get content from buffer: " .. tostring(buf_content))
		return
	end
	local password = vim.fn.inputsecret("Password: ")

	local ok_decrypt, result = fcryptc.decrypt_buf(buf_content, password)
	if not ok_decrypt then
		logs.error("\nDecryption failed: " .. tostring(result))
		return
	end
	if result == nil then
		logs.error("\nDecrypted content is nil: " .. tostring(result))
		return
	end

	buffer.set_current_buffer_content(result)
end

return M
