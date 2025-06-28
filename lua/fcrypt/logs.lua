-- lua/fcrypt/logs.lua

local floating = require("fcrypt.floating")

local M = {
	_floating = {},
	_log_lines = {},
}

local function append(msg)
	local lines = type(msg) == "table" and msg or vim.split(msg, "\n", {})
	vim.list_extend(M._log_lines, lines)
	M.refresh()
end

local function timestamp()
	return os.date("%Y-%m-%d %H:%M:%S")
end

function M.refresh()
	local buf = M._floating.buf
	if not buf or not vim.api.nvim_buf_is_valid(buf) then
		return
	end

	vim.bo[buf].modifiable = true
	vim.bo[buf].readonly = false
	vim.api.nvim_buf_set_lines(buf, 0, -1, false, M._log_lines)
	vim.bo[buf].modifiable = false
	vim.bo[buf].readonly = true

	local win = M._floating.win
	if win and vim.api.nvim_win_is_valid(win) then
		local line_count = vim.api.nvim_buf_line_count(buf)
		vim.api.nvim_win_set_cursor(win, { line_count, 0 })
	end
end

function M.info(msg)
	append(string.format("%s [INFO] %s", timestamp(), msg))
	-- vim.notify(msg, vim.log.levels.INFO)
end

function M.warn(msg)
	append(string.format("%s [WARN] %s", timestamp(), msg))
	-- vim.notify(msg, vim.log.levels.WARN)
end

function M.error(msg)
	append(string.format("%s [ERROR] %s", timestamp(), msg))
	vim.notify(msg, vim.log.levels.ERROR)
end

function M.present()
	local buf, win = floating.present_bottom({
		buf = M._floating.buf,
		filetype = "log",
		buftype = "nofile",
		bufhidden = "hide",
	})

	M._floating.buf = buf
	M._floating.win = win

	M.refresh()

	vim.api.nvim_buf_set_keymap(buf, "n", "q", ":close<CR>", { noremap = true, silent = true })
end

return M
