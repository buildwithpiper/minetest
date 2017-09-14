--
-- Mod channels experimental handlers
--
local mod_channel = core.mod_channel_join("experimental_preview")

core.register_on_modchannel_message(function(channel, sender, message)
	print("[minimal][modchannels] Server received message `" .. message
			.. "` on channel `" .. channel .. "` from sender `" .. sender .. "`")

	if mod_channel:is_writeable() then
		mod_channel:send("experimental answers to preview")
		mod_channel:leave()
	end
end)

print("[minimal][modchannels] Code loaded!")
