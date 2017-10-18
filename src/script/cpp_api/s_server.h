/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#pragma once

#include "irr_v3d.h"
#include "cpp_api/s_base.h"
#include <set>
#include "util/pointedthing.h"

class ScriptApiServer
		: virtual public ScriptApiBase
{
public:
	// Calls on_chat_message handlers
	// Returns true if script handled message
	bool on_chat_message(const std::string &name, const std::string &message);
	bool on_plugin_message(const std::string &name, const std::string &plugin, const std::string &message);
	// Calls on_shutdown handlers
	void on_shutdown();

	void on_interactnodes(v3s16 p, ServerActiveObject *puncher);

	/* auth */
	bool getAuth(const std::string &playername,
			std::string *dst_password,
			std::set<std::string> *dst_privs);
	void createAuth(const std::string &playername,
			const std::string &password);
	bool setPassword(const std::string &playername,
			const std::string &password);
private:
	void getAuthHandler();
	void readPrivileges(int index, std::set<std::string> &result);
};
