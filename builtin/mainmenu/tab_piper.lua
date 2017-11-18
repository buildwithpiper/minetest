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
local inventor_names = dofile(core.get_mainmenu_path()..DIR_DELIM.."names.lua")
local playername = core.settings:get("piper_name")
local tab
local data = {
	status = "Detecting Internet Connection...",
}
local need_restart = false
local basepath = core.get_builtin_path()
local update = {finished=false, active=true, status=""}

local function choose_new_name()
	repeat
		playername = inventor_names[math.random(#inventor_names)]
	until #playername < 20
	core.settings:set("piper_name", playername)
	return playername
end

if not playername then choose_new_name() end

local function draw_grid()
	for i=-10,30,0.25 do
		for j=-10,30,0.25 do
			if i == 0 and j == 0 then
				e("box[" .. i .. "," .. j .. ";0.05,0.05;#FFFF00FF]")
			else
				e("box[" .. i .. "," .. j .. ";0.05,0.05;#FF0000FF]")
			end
		end
	end
end

local function get_formspec(tabview, name, tabdata)
	local retval = {}
	local function e(s) table.insert(retval, s) end


	local list = {}
	local n = 0
	for line in string.gmatch(update.status, "[^\n]+") do
		n = n + 1
		table.insert(list, core.formspec_escape(line))
	end

	-- Status
	local status = "Connected."
	local xpos = '7.6'
	if not update.finished and update.started then
        e("container[5,1]")
        e("textlist[-0.25,-0.1;7,2.5;us;" .. table.concat(list, ',') .. ';' .. n .. ';false]')
        e("container_end[]")
		status = "Connecting..."
	elseif not data.name then
		local xpos = '7.2'
		status = "Connecting Failed"
	end
	e("label["..xpos..",3.88;" .. status .. ']')

	
	local function i(img)
		return core.formspec_escape(defaulttexturedir .. "piper" .. DIR_DELIM .. img)
	end

		e("background[-0.2,-0.25;12.4,6.3;" .. i("background.png") .. "]")			
        --21 - 3.7
		e("label[" .. 1.9 - (#playername/2) * (3.2/22.0) .. ",4.08;" .. playername .. "]")
		e("image_button[0.25,4.95;3.1,0.6;"..i("choosenewname.png")..";choosename;]")

		e("image_button[5.2,0.22;1.0,0.65;"..i("dev-lair.png")..";dev;]")
		e("image[7.2,0.20;2.1,0.60;"..i(data.name and "online.png" or "offline.png")..']')
		e("image_button[10.0,0.21;1.5,0.69;"..i("exittomenu.png")..";exit;]")

		if not data.name then 
			e("button[6.8,4.5;3,1.5;tryagain;Try Again]")
		elseif update.finished then
			e("button[6.8,4.5;3,1.5;join;Join]")
		else
			e("button[6.8,4.5;3,1.5;trying;Trying to Join]")
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

local version = tonumber(read_file(basepath .. '..' .. DIR_DELIM .. 'version.txt')) or 0
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
local dev_button_count = 0
local function main_button_handler(tabview, fields, name, tabdata)
	print(dump({tabview=tabview, fields=fields, name=name, tabdata=tabdata}))
	if fields.dev then
		if dev_button_count >= 2 then
			tab.enable_regular_options()
			ui.update()
		else
			dev_button_count = dev_button_count + 1
		end
	end

	if fields.tryagain then
		get_config()
	end

	if fields.choosename then
		choose_new_name()
		ui.update()
	end

	if fields.exit then
		os.execute('sudo reboot')
	end

	if fields.join then
		gamedata.playername = core.settings:get("piper_name")
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
