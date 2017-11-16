--Minetest
--Copyright (C) 2014 sapier
--
--This program is free software; you can redistribute it and/or modify
--it under the terms of the GNU Lesser General Public License as published by
--the Free Software Foundation; either version 2.1 of the License, or
--(at your option) any later version.
--
--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU Lesser General Public License for more details.
--
--You should have received a copy of the GNU Lesser General Public License along
--with this program; if not, write to the Free Software Foundation, Inc.,
--51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

--------------------------------------------------------------------------------
local tab
local data = {
	status = "Detecting Internet Connection..."
}
local need_restart = false
local basepath = core.get_builtin_path()
local update = {finished=false, active=true, status=""}
local function get_formspec(tabview, name, tabdata)
	local retval = {}
	local function e(s) table.insert(retval, s) end


	local list = {}
	local n = 0
	for line in string.gmatch(update.status, "[^\n]+") do
		n = n + 1
		table.insert(list, core.formspec_escape(line))
	end
	-- Connect
	e("label[6,2;" .. core.formspec_escape(data.status) .. ']')

	if not update.finished and update.started then
		e("container[-3,0]")
		e("textlist[0,0;10,5;us;" .. table.concat(list, ',') .. ';' .. n .. ';false]')
		e("container_end[]")
	end

	if update.finished then
		e("button[6,2;6,2;btn_pip_connect;" .. fgettext("Connect") .. " to " .. data.name .. "]")
	elseif not data.name then
		e("button[4,4;6,2;btn_pip_test;" .. fgettext("Try Again...") .. "]")
	end
	
	e("button[11.25,4.5;1,2;btn_pip_developer;" .. fgettext("D") .. "]")

	if not data.name then
		e("background[-0.2,-0.25;5,5;" .. core.formspec_escape(defaulttexturedir .. "piper" .. DIR_DELIM .. "menu_offline.png") .. "]")
	elseif update.finished then
		e("background[-0.2,-0.25;12.4,6;" .. core.formspec_escape(defaulttexturedir .. "piper" .. DIR_DELIM .. "menu.png") .. "]")			
	end

	for i=-10,30,0.25 do
		for j=-10,30,0.25 do
			if i == 0 and j == 0 then
				e("box[" .. i .. "," .. j .. ";0.05,0.05;#FFFF00FF]")
			else
				e("box[" .. i .. "," .. j .. ";0.05,0.05;#FF0000FF]")
			end
		end
	end
	return table.concat(retval, '\n')
end

local function read_file(where)
	local f = io.open(where, 'r')
	if not f then return nil end
	local str = f:read('*all')
	f:close()
	return str
end

local version = tonumber(read_file(basepath .. '..' .. DIR_DELIM .. 'version.txt'))
print("VERSION IS", version)

local function do_update()
	local update = {finished=false, started=true, status="Starting...."}
	local where = os.tmpname()
	core.handle_async(function(params)
		print('bash -c "' .. params.basepath .. '..' .. DIR_DELIM .. 'update.sh" > ' .. params.where .. '"')
		os.execute('bash -c "' .. params.basepath .. '..' .. DIR_DELIM .. 'update.sh > ' .. params.where .. '"')
	end, {where=where, basepath=basepath}, function()
		update.finished = true
		if need_restart then
			os.exit()
		end
	end)
	local function read()
		core.handle_async(read_file, where, function(data)
			if string.match(data, 'bin/minetest') then need_restart = true end
			if string.match(data, 'update.sh') then need_restart = true end
			update.status = data
			ui.update()
			read()
		end)
	end
	read()

	return update

end

local function get_config()

	core.handle_async(
		function(params)
			local f = io.popen("curl --connect-timeout 4 http://config.swarm.playpiper.com/config -d version=" .. params.version)
			local raw = f:read('*all')
			local okay, data = pcall(minetest.parse_json, raw)
			if not okay then return nil end
			f:close()
			return data
		end,
		{version=version},
		function(result)
			if result == nil then
				data.status = "No internet detected!" .. math.random(100)
				data.okay = false
			else
				result.status = "Server found!"
				data = result
				data.okay = true
			end
			print(dump(data))
			if data.okay and data.should_update then
				update = do_update()
			else
				update.finished = true
			end
			ui.update()
		end
	)

end

mm_texture.clear("header")
get_config()


--------------------------------------------------------------------------------
local function main_button_handler(tabview, fields, name, tabdata)
	print(dump({tabview=tabview, fields=fields, name=name, tabdata=tabdata}))
	if fields.btn_pip_developer then
		tab.enable_regular_options()
		ui.update()
	end

	if fields.btn_pip_test then
		get_config()
	end

	if fields.btn_pip_connect then
		gamedata.playername = "PiperBot_" .. math.random(100)
		gamedata.password   = ""
		gamedata.address    = data.server
		gamedata.port       = data.port
		gamedata.selected_world = 0
		core.start()
		return true
	end
	return false
end

local function on_change(type, old_tab, new_tab)
	if type == "LEAVE" then return end
	asyncOnlineFavourites()
end

--------------------------------------------------------------------------------
tab = {
	name = "online",
	caption = fgettext("PiperCraft : Alpha"),
	cbf_formspec = get_formspec,
	cbf_button_handler = main_button_handler,
	on_change = on_change
}

return tab