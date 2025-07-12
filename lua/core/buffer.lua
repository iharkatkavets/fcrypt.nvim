-- core/buffer.lua

local M = {}

function M.get_current_buffer_content()
	local lines = vim.api.nvim_buf_get_lines(0, 0, -1, false)
	return table.concat(lines, "\n")
end

function M.get_current_buffer_file_content()
	local path = vim.api.nvim_buf_get_name(0)
	if path == "" then
		return false, "Buffer is not associated with a file."
	end
	local f = assert(io.open(path, "rb"))
	local file_content = f:read("*a")
	f:close()
	return true, file_content
end

function M.set_current_buffer_content(content)
	if type(content) ~= "string" then
		error("set_current_buffer_content: content must be a string, got " .. type(content))
	end

	local lines = vim.split(content, "\n", { plain = true })
	local buf = vim.api.nvim_get_current_buf()
	vim.api.nvim_buf_call(buf, function()
		vim.api.nvim_buf_set_lines(buf, 0, -1, false, lines)
	end)
end

return M
