-- lua/fcrypt/path.lua

local logs = require("fcrypt.logs")

local M = {}

function M.add_cpath_for_native_module(module_filename)
	local runtime_files = vim.api.nvim_get_runtime_file("lua/fcrypt/" .. module_filename, false)

	if #runtime_files == 0 then
		logs.error(string.format("fcrypt.nvim: Native module '%s' not found in runtimepath", module_filename))
		return
	end

	local module_dir = vim.fn.fnamemodify(runtime_files[1], ":h")
	local cpath_entry = module_dir .. "/?.so"

	if not package.cpath:find(vim.pesc(cpath_entry), 1, true) then
		package.cpath = package.cpath .. ";" .. cpath_entry
	end
end

return M
