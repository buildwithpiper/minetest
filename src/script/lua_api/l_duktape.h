/*
Minetest
Copyright (C) 2013 kwolekr, Ryan Kwolek <kwolekr@minetest.net>

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

#include <map>
#include "irr_v3d.h"
#include "lua_api/l_base.h"
#include "duktape.h"

/*
  VoxelManip
 */
class LuaDuktape : public ModApiBase
{
public:
	static void Register(lua_State *L);
	static void transfer(duk_context *ctx, lua_State *L, int idx);
	static void transfer(lua_State *L, duk_context *ctx, int idx);
private:
	static const char className[];
	static const luaL_Reg methods[];

	static int gc_object(lua_State *L);
	static int l_run(lua_State *L);
	static int l_peval(lua_State *L);
	static int l_push(lua_State *L);
	static int l_get(lua_State *L);
	static int l_pop(lua_State *L);

	static int l_put_global_string(lua_State *L);
	static int l_push_global_object(lua_State *L);

	duk_context *ctx;
	lua_State *L;

	LuaDuktape(lua_State *L);
	~LuaDuktape();

	// LuaVoxelManip()
	// Creates a LuaVoxelManip and leaves it on top of stack
	static int create_object(lua_State *L);

	static LuaDuktape *checkobject(lua_State *L, int narg);

	
};
