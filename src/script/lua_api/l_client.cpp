/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2017 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "l_client.h"
#include "chatmessage.h"
#include "client.h"
#include "settings.h"
#include "client/clientevent.h"
#include "clientenvironment.h"
#include "common/c_content.h"
#include "common/c_converter.h"
#include "cpp_api/s_base.h"
#include "gettext.h"
#include "l_internal.h"
#include "lua_api/l_item.h"
#include "lua_api/l_nodemeta.h"
#include "mainmenumanager.h"
#include "map.h"
#include "util/string.h"
#include "nodedef.h"
#include "client/renderingengine.h"

int ModApiClient::l_get_current_modname(lua_State *L)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_CURRENT_MOD_NAME);
	return 1;
}

// get_last_run_mod()
int ModApiClient::l_get_last_run_mod(lua_State *L)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_CURRENT_MOD_NAME);
	const char *current_mod = lua_tostring(L, -1);
	if (current_mod == NULL || current_mod[0] == '\0') {
		lua_pop(L, 1);
		lua_pushstring(L, getScriptApiBase(L)->getOrigin().c_str());
	}
	return 1;
}

// set_last_run_mod(modname)
int ModApiClient::l_set_last_run_mod(lua_State *L)
{
	if (!lua_isstring(L, 1))
		return 0;

	const char *mod = lua_tostring(L, 1);
	getScriptApiBase(L)->setOriginDirect(mod);
	lua_pushboolean(L, true);
	return 1;
}

// print(text)
int ModApiClient::l_print(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string text = luaL_checkstring(L, 1);
	rawstream << text << std::endl;
	return 0;
}

// display_chat_message(message)
int ModApiClient::l_display_chat_message(lua_State *L)
{
	if (!lua_isstring(L, 1))
		return 0;

	std::string message = luaL_checkstring(L, 1);
	getClient(L)->pushToChatQueue(new ChatMessage(utf8_to_wide(message)));
	lua_pushboolean(L, true);
	return 1;
}

// send_chat_message(message)
int ModApiClient::l_send_chat_message(lua_State *L)
{
	if (!lua_isstring(L, 1))
		return 0;

	// If server disabled this API, discard
	if (getClient(L)->checkCSMFlavourLimit(CSMFlavourLimit::CSM_FL_CHAT_MESSAGES))
		return 0;

	std::string message = luaL_checkstring(L, 1);
	getClient(L)->sendChatMessage(utf8_to_wide(message));
	return 0;
}

// clear_out_chat_queue()
int ModApiClient::l_clear_out_chat_queue(lua_State *L)
{
	getClient(L)->clearOutChatQueue();
	return 0;
}

// get_player_names()
int ModApiClient::l_get_player_names(lua_State *L)
{
	const std::list<std::string> &plist = getClient(L)->getConnectedPlayerNames();
	lua_createtable(L, plist.size(), 0);
	int newTable = lua_gettop(L);
	int index = 1;
	std::list<std::string>::const_iterator iter;
	for (iter = plist.begin(); iter != plist.end(); ++iter) {
		lua_pushstring(L, (*iter).c_str());
		lua_rawseti(L, newTable, index);
		index++;
	}
	return 1;
}

// show_formspec(formspec)
int ModApiClient::l_show_formspec(lua_State *L)
{
	if (!lua_isstring(L, 1) || !lua_isstring(L, 2))
		return 0;

	ClientEvent *event = new ClientEvent();
	event->type = CE_SHOW_LOCAL_FORMSPEC;
	event->show_formspec.formname = new std::string(luaL_checkstring(L, 1));
	event->show_formspec.formspec = new std::string(luaL_checkstring(L, 2));
	getClient(L)->pushToEventQueue(event);
	lua_pushboolean(L, true);
	return 1;
}

// send_respawn()
int ModApiClient::l_send_respawn(lua_State *L)
{
	getClient(L)->sendRespawn();
	return 0;
}

// Change map()
int ModApiClient::l_change_map(lua_State *L)
{
	// Stops badly written Lua code form causing boot loops
	if (getClient(L)->isShutdown()) {
		lua_pushboolean(L, false);
		return 1;
	}
	std::string mapname = luaL_checkstring(L,1);
	g_gamecallback->change_map(mapname);
	lua_pushboolean(L, true);
	return 1;
}

// disconnect()
int ModApiClient::l_disconnect(lua_State *L)
{
	// Stops badly written Lua code form causing boot loops
	if (getClient(L)->isShutdown()) {
		lua_pushboolean(L, false);
		return 1;
	}

	g_gamecallback->disconnect();
	lua_pushboolean(L, true);
	return 1;
}

// gettext(text)
int ModApiClient::l_gettext(lua_State *L)
{
	std::string text = strgettext(std::string(luaL_checkstring(L, 1)));
	lua_pushstring(L, text.c_str());

	return 1;
}

// get_node(pos)
// pos = {x=num, y=num, z=num}
int ModApiClient::l_get_node_or_nil(lua_State *L)
{
	// pos
	v3s16 pos = read_v3s16(L, 1);

	// Do it
	bool pos_ok;
	MapNode n = getClient(L)->getNode(pos, &pos_ok);
	if (pos_ok) {
		// Return node
		pushnode(L, n, getClient(L)->ndef());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int ModApiClient::l_get_wielded_item(lua_State *L)
{
	Client *client = getClient(L);

	Inventory local_inventory(client->idef());
	client->getLocalInventory(local_inventory);

	InventoryList *mlist = local_inventory.getList("main");

	if (mlist && client->getPlayerItem() < mlist->getSize()) {
		LuaItemStack::create(L, mlist->getItem(client->getPlayerItem()));
	} else {
		LuaItemStack::create(L, ItemStack());
	}
	return 1;
}

// get_meta(pos)
int ModApiClient::l_get_meta(lua_State *L)
{
	v3s16 p = read_v3s16(L, 1);
	NodeMetadata *meta = getClient(L)->getEnv().getMap().getNodeMetadata(p);
	NodeMetaRef::createClient(L, meta);
	return 1;
}

int ModApiClient::l_sound_play(lua_State *L)
{
	ISoundManager *sound = getClient(L)->getSoundManager();

	SimpleSoundSpec spec;
	read_soundspec(L, 1, spec);
	float gain = 1.0f;
	float pitch = 1.0f;
	bool looped = false;
	s32 handle;

	if (lua_istable(L, 2)) {
		getfloatfield(L, 2, "gain", gain);
		getfloatfield(L, 2, "pitch", pitch);
		getboolfield(L, 2, "loop", looped);

		lua_getfield(L, 2, "pos");
		if (!lua_isnil(L, -1)) {
			v3f pos = read_v3f(L, -1) * BS;
			lua_pop(L, 1);
			handle = sound->playSoundAt(
					spec.name, looped, gain * spec.gain, pos, pitch);
			lua_pushinteger(L, handle);
			return 1;
		}
	}

	handle = sound->playSound(spec.name, looped, gain * spec.gain, 0.0f, pitch);
	lua_pushinteger(L, handle);

	return 1;
}

int ModApiClient::l_sound_stop(lua_State *L)
{
	u32 handle = luaL_checkinteger(L, 1);

	getClient(L)->getSoundManager()->stopSound(handle);

	return 0;
}

// get_server_info()
int ModApiClient::l_get_server_info(lua_State *L)
{
	Client *client = getClient(L);
	Address serverAddress = client->getServerAddress();
	lua_newtable(L);
	lua_pushstring(L, client->getAddressName().c_str());
	lua_setfield(L, -2, "address");
	lua_pushstring(L, serverAddress.serializeString().c_str());
	lua_setfield(L, -2, "ip");
	lua_pushinteger(L, serverAddress.getPort());
	lua_setfield(L, -2, "port");
	lua_pushinteger(L, client->getProtoVersion());
	lua_setfield(L, -2, "protocol_version");
	return 1;
}

// get_item_def(itemstring)
int ModApiClient::l_get_item_def(lua_State *L)
{
	IGameDef *gdef = getGameDef(L);
	assert(gdef);

	IItemDefManager *idef = gdef->idef();
	assert(idef);

	if (getClient(L)->checkCSMFlavourLimit(CSMFlavourLimit::CSM_FL_READ_ITEMDEFS))
		return 0;

	if (!lua_isstring(L, 1))
		return 0;

	const std::string &name(lua_tostring(L, 1));
	if (!idef->isKnown(name))
		return 0;
	const ItemDefinition &def = idef->get(name);

	push_item_definition_full(L, def);

	return 1;
}

// get_node_def(nodename)
int ModApiClient::l_get_node_def(lua_State *L)
{
	IGameDef *gdef = getGameDef(L);
	assert(gdef);

	INodeDefManager *ndef = gdef->ndef();
	assert(ndef);

	if (!lua_isstring(L, 1))
		return 0;

	if (getClient(L)->checkCSMFlavourLimit(CSMFlavourLimit::CSM_FL_READ_NODEDEFS))
		return 0;

	const std::string &name = lua_tostring(L, 1);
	const ContentFeatures &cf = ndef->get(ndef->getId(name));
	if (cf.name != name) // Unknown node. | name = <whatever>, cf.name = ignore
		return 0;

	push_content_features(L, cf);

	return 1;
}

int ModApiClient::l_take_screenshot(lua_State *L)
{
	Client *client = getClient(L);
	client->makeScreenshot();
	return 0;
}

int ModApiClient::l_get_privilege_list(lua_State *L)
{
	const Client *client = getClient(L);
	lua_newtable(L);
	for (const std::string &priv : client->getPrivilegeList()) {
		lua_pushboolean(L, true);
		lua_setfield(L, -2, priv.c_str());
	}
	return 1;
}

// get_builtin_path()
int ModApiClient::l_get_builtin_path(lua_State *L)
{
	lua_pushstring(L, BUILTIN_MOD_NAME ":");
	return 1;
}

// send_plugin_message(plugin, data)
int ModApiClient::l_send_plugin_message(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *plugin = luaL_checkstring(L, 1);
	const char *text = luaL_checkstring(L, 2);

	Client *client = getClient(L);
	client->sendPluginMessage(plugin, text);
	return 0;
}

// Free the cursor
int ModApiClient::l_free_cursor(lua_State *L)
{
	lua_pushboolean(L, true);

	return 1;
}

// Lock the cursor
int ModApiClient::l_lock_cursor(lua_State *L)
{
	lua_pushboolean(L,false);

	return 1;
}

// Send arbitrary keys to Irrlicht
int ModApiClient::l_send_keys(lua_State *L)
{
	std::string key = luaL_checkstring(L, 1);
	bool down = lua_toboolean(L,2);

    irr::SEvent myEvent;

    if(key == "space")
            myEvent.KeyInput.Key = irr::KEY_SPACE;
    else if(key == "escape")
            myEvent.KeyInput.Key = irr::KEY_ESCAPE;
    else if(key == "a")
            myEvent.KeyInput.Key = irr::KEY_KEY_A;
    else if(key == "d")
            myEvent.KeyInput.Key = irr::KEY_KEY_D;
    else if(key == "s")
            myEvent.KeyInput.Key = irr::KEY_KEY_S;
    else if(key == "w")
            myEvent.KeyInput.Key = irr::KEY_KEY_W;
    else if(key == "i")
            myEvent.KeyInput.Key = irr::KEY_KEY_I;

	if (key.size() == 1)
		myEvent.KeyInput.Char = key[0];

    myEvent.EventType = irr::EET_KEY_INPUT_EVENT;
    myEvent.KeyInput.PressedDown = down;

    irr::IrrlichtDevice* dev = RenderingEngine::get_raw_device();
    dev->postEventFromUser(myEvent);
    
	return 0;
}

// Get the window id if we're on Linux, else 0
int ModApiClient::l_get_window_id(lua_State *L)
{
	unsigned long id = RenderingEngine::getWindowId();
	lua_pushinteger(L, id);
	return 1;
}

// Change client setting
int ModApiClient::l_set_setting(lua_State *L)
{
	std::string key = luaL_checkstring(L, 1);
	std::string value = luaL_checkstring(L, 2);
	g_settings->set(key, value);
	return 0;
}

void ModApiClient::Initialize(lua_State *L, int top)
{
	API_FCT(get_current_modname);
	API_FCT(print);
	API_FCT(display_chat_message);
	API_FCT(send_chat_message);
	API_FCT(clear_out_chat_queue);
	API_FCT(get_player_names);
	API_FCT(set_last_run_mod);
	API_FCT(get_last_run_mod);
	API_FCT(show_formspec);
	API_FCT(send_respawn);
	API_FCT(gettext);
	API_FCT(get_node_or_nil);
	API_FCT(get_wielded_item);
	API_FCT(change_map);
	API_FCT(disconnect);
	API_FCT(get_meta);
	API_FCT(sound_play);
	API_FCT(sound_stop);
	API_FCT(get_server_info);
	API_FCT(get_item_def);
	API_FCT(get_node_def);
	API_FCT(take_screenshot);
	API_FCT(get_privilege_list);
	API_FCT(get_builtin_path);
	API_FCT(send_plugin_message);
	API_FCT(free_cursor);
	API_FCT(lock_cursor);
	API_FCT(send_keys);
	API_FCT(get_window_id);
	API_FCT(set_setting);
}
