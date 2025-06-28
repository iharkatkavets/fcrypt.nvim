-- lua/fcrypt/floating.lua

local M = {}

function M.create_buf_if_needed(opts)
	if opts.buf and vim.api.nvim_buf_is_valid(opts.buf) then
		return opts.buf
	else
		local buf = vim.api.nvim_create_buf(false, true)

		if opts.filetype then
			vim.bo[buf].filetype = opts.filetype
		end
		if opts.buftype then
			vim.bo[buf].buftype = opts.buftype
		end
		if opts.bufhidden then
			vim.bo[buf].bufhidden = opts.bufhidden
		end

		return buf
	end
end

function M.present_bottom(opts)
	opts = opts or {}
	local width = opts.width or math.floor(vim.o.columns)
	local height = opts.height or math.floor(vim.o.lines * 0.5)
	local xrow = vim.o.lines - height - 1
	local ycol = math.floor((vim.o.columns - width) / 2)

	local buf = M.create_buf_if_needed(opts)

	local win_opts = {
		style = "minimal",
		relative = "editor",
		width = width,
		height = height,
		row = xrow,
		col = ycol,
		border = "rounded",
	}

	local win = vim.api.nvim_open_win(buf, true, win_opts)
	vim.api.nvim_buf_set_keymap(buf, "n", "q", ":close<CR>", { noremap = true, silent = true })

	if buf and vim.api.nvim_buf_is_valid(buf) then
		vim.api.nvim_create_autocmd("WinLeave", {
			buffer = buf,
			callback = function()
				if win and vim.api.nvim_win_is_valid(win) then
					vim.api.nvim_win_close(win, true)
					win = -1
				end
			end,
		})
	end
	return buf, win
end

return M
