-- fcrypt/native.lua

local logs = require("fcrypt.logs")

local path_utils = require("fcrypt.path")
path_utils.add_cpath_for_native_module("fcrypt_lua_mod.so")

local ok_native, native = pcall(require, "fcrypt_lua_mod")
if not ok_native then
	logs.error("Failed to load native module: " .. tostring(native))
	native = nil
end

local M = {}

function M.get_fcrypt_version()
	if native == nil then
		return false, "Native module is not available. Check logs"
	end

	local ok, result = pcall(native.get_fcrypt_version)
	if not ok then
		logs.error("Failed to get fcrypt version: " .. tostring(result))
		return false, "Failed to get fcrypt version: " .. tostring(result)
	end

	logs.info("fcrypt version is" .. result)
	return true, result
end

function M.encrypt_buf(content, password, hint)
	if native == nil then
		return false, "Native module is not available. Check logs"
	end

	local ok, result = pcall(native.encrypt_buf, content, password, hint)
	if not ok then
		logs.error("Failed to encrypt: " .. tostring(result))
		return false, "Failed to encrypt: " .. tostring(result)
	end

	return true, result
end

function M.decrypt_buf(content, password)
	if native == nil then
		return false, "Native module is not available. Check logs"
	end

	local ok, result = pcall(native.decrypt_buf, content, password)
	if not ok then
		logs.error("Decryption failed: " .. tostring(result))
		return false, "Decryption failed: " .. tostring(result)
	end

	return true, result
end

return M
