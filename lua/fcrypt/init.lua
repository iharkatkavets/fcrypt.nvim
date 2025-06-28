-- lua/fcrypt/init.lua

local logs = require("fcrypt.logs")
local decrypt = require("fcrypt.decrypt")
local encrypt = require("fcrypt.encrypt")
local fcryptc = require("fcrypt.native")

local M = {}

local create_commands = function()
	vim.api.nvim_create_user_command("FCryptDecryptBuf", function()
		decrypt.decrypt_buf()
	end, {
		desc = "Decrypt current buffer",
	})

	vim.api.nvim_create_user_command("FCryptEncryptBuf", function()
		encrypt.encrypt()
	end, {
		desc = "Encrypt current buffer",
	})

	vim.api.nvim_create_user_command("FCryptLogs", function()
		logs.present()
	end, {
		desc = "Open logs",
	})

	vim.api.nvim_create_user_command("FCryptVersion", function()
		local ok, result = fcryptc.get_fcrypt_version()
		if not ok then
			vim.notify("fcryptc module is not available", vim.log.levels.ERROR)
		else
			vim.notify("fcrypt version is" .. result, vim.log.levels.INFO)
		end
	end, {
		desc = "Print fcrypt version",
	})
end

M.setup = function(_)
	create_commands()
end

return M
