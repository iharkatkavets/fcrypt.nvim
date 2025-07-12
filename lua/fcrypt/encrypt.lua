-- lua/fcrypt/encrypt.lua

local logs = require("fcrypt.logs")
local buffer = require("core.buffer")
local fcryptc = require("fcrypt.native")

local M = {}

local function ask_for_input_parameters(callback)
	local password1 = vim.fn.inputsecret("Password: ")
	if not password1 or password1 == "" then
		logs.warning("\nNo password entered. Aborting.")
		return
	end

	local password2 = vim.fn.inputsecret("Repeat password: ")
	if password1 ~= password2 then
		logs.error("\nPasswords do not match. Aborting.")
		return
	end

	vim.ui.input({ prompt = "Password hint (optional): " }, function(hint)
		callback(password1, hint)
	end)
end

function M.encrypt()
	local content = buffer.get_current_buffer_content()

	ask_for_input_parameters(function(password, hint)
		local ok_encrypt, result = fcryptc.encrypt_buf(content, password, hint)
		if not ok_encrypt then
			logs.error("Encryption failed: " .. tostring(result))
			return
		end

		buffer.set_current_buffer_content(result)
	end)
end

return M
