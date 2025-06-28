-- core/buffer.lua

local M = {}

function M.get_current_buffer_content()
	local lines = vim.api.nvim_buf_get_lines(0, 0, -1, false)
	return table.concat(lines, "\n")
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
