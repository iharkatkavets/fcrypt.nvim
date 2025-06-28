-- core/string.lua

local M = {}

function M.write_string_to_file(path, content)
	if type(content) ~= "string" then
		return false, "Content must be a string, got: " .. tostring(type(content))
	end

	local file, err = io.open(path, "w")
	if not file then
		return false, "Failed to open file: " .. tostring(err)
	end

	file:write(content)
	file:close()
	return true
end

return M
