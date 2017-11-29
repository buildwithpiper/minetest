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


#include "lua_api/l_duktape.h"
#include "lua_api/l_internal.h"
#include "duktape.h"

// garbage collector
int LuaDuktape::gc_object(lua_State *L)
{
	LuaDuktape *o = *(LuaDuktape **)(lua_touserdata(L, 1));
	delete o;
	return 0;
}

static void duk_lua_fatal(void *udata, const char *msg) {
	lua_pushstring((lua_State *)udata, msg);
	lua_error((lua_State *)udata);
}

static duk_ret_t duk_lua_call(duk_context *ctx) {
	duk_push_current_function(ctx);
	
	duk_get_prop_string(ctx, -1, "\xff" "lua");
	lua_State * L = (lua_State*)duk_get_pointer(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "\xff" "refrence");
	int r = duk_get_uint(ctx, -1);
	duk_pop_2(ctx);

	int top = lua_gettop(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, r);
	duk_idx_t nargs = duk_get_top(ctx);
	for ( int i = 0; i < nargs; ++i ) {
		LuaDuktape::transfer(ctx, L, i);
	}

	duk_thread_state state;
	duk_suspend(ctx, &state);
	lua_call(L, nargs, LUA_MULTRET);
	duk_resume(ctx, &state);
	int ntop = lua_gettop(L);
	int nresults = ntop - top;
	//printf("Transfered %d (%d - %d) results...", nresults, top, ntop);
	if ( nresults == 1 ) {
		LuaDuktape::transfer(L, ctx, ntop);
	} else if ( nresults > 1 ) {
		duk_push_array(ctx);
		for ( int i = 0; i < nresults; ++i ) {
			LuaDuktape::transfer(L, ctx, top + 1 + i);
			duk_put_prop_index(ctx, -2, i);
		}
	}

	lua_pop(L, nresults);
	return nresults > 0 ? 1 : 0;
}

static duk_ret_t duk_lua_call_finalize(duk_context *ctx) {
	duk_get_prop_string(ctx, -1, "\xff" "lua");
	lua_State * L = (lua_State*)duk_get_pointer(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "\xff" "refrence");
	int r = duk_get_uint(ctx, -1);
	duk_pop_2(ctx);

	luaL_unref(L, LUA_REGISTRYINDEX, r);
	return 0;
}


static int duk_lua_meta_gc(lua_State *L)
{
	int idx = lua_tonumber(L, lua_upvalueindex(1));
	duk_context* ctx = *(duk_context **)lua_touserdata(L,  lua_upvalueindex(2));
	
	duk_push_global_stash(ctx);
	duk_del_prop_index(ctx, -1, idx);
	duk_pop(ctx);
	return 0;
}

static int duk_lua_meta_index(lua_State *L)
{
	duk_context* ctx = *(duk_context **)lua_touserdata(L, lua_upvalueindex(1));
	void * lud = *(void **)lua_touserdata(L, lua_upvalueindex(2));
	duk_push_heapptr(ctx, lud);
	duk_dup(ctx, -1);
	duk_pop(ctx);
	duk_get_prop_string(ctx, -1, lua_tostring(L, 2));
	LuaDuktape::transfer(ctx, L, -1);
	duk_pop_2(ctx);
	return 1;
}

static int duk_lua_meta_tostring(lua_State *L)
{
	duk_context* ctx = *(duk_context **)lua_touserdata(L, lua_upvalueindex(1));
	void * lud = *(void **)lua_touserdata(L, lua_upvalueindex(2));
	duk_push_heapptr(ctx, lud);
	lua_pushstring(L, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	return 1;
}


static int duk_lua_closure_call(lua_State *L)
{
	duk_context* ctx = *(duk_context **)lua_touserdata(L, lua_upvalueindex(1));
	void * lud = *(void **)lua_touserdata(L, lua_upvalueindex(2));
	duk_push_heapptr(ctx, lud);
	int nargs = lua_gettop(L);
	for ( int i = 1; i <= nargs; ++i ) {
		LuaDuktape::transfer(L, ctx, i);
	}
	duk_call(ctx, nargs);

	//If an array is returned, blow that out int multiple return values
	if ( duk_is_array(ctx, -1) ) {
		int len = duk_get_length(ctx, -1);
		for ( int i = 0; i < len; ++i ) {
			duk_get_prop_index(ctx, -1, i);
			LuaDuktape::transfer(ctx, L, -1);
			duk_pop(ctx);
		}
		return len;
	} else {
		LuaDuktape::transfer(ctx, L, -1);
		duk_pop(ctx);
		return 1;
	}

}

static duk_ret_t duk_lua_proxy_get(duk_context *ctx) {
	duk_get_prop_string(ctx, -1, "\xff" "lua");
	lua_State * L = (lua_State*)duk_get_pointer(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "\xff" "refrence");
	int r = duk_get_uint(ctx, -1);
	duk_pop_2(ctx);

	lua_rawgeti(L, LUA_REGISTRYINDEX, r);
	lua_getfield(L, -1, duk_safe_to_string(ctx, -1));
	LuaDuktape::transfer(L, ctx, -1);
	lua_pop(L, 1);	
	return 1;
}

static duk_ret_t duk_lua_proxy_ownkeys(duk_context *ctx) {
	duk_get_prop_string(ctx, 0, "\xff" "lua");
	lua_State * L = (lua_State*)duk_get_pointer(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "\xff" "refrence");
	int r = duk_get_uint(ctx, -1);
	duk_pop_2(ctx);

	duk_push_array(ctx);
	int count = 0;
	lua_rawgeti(L, LUA_REGISTRYINDEX, r);
	lua_pushnil(L);  /* first key */
	while (lua_next(L, -2) != 0) {
		lua_pop(L,1);
		duk_push_string(ctx, lua_tostring(L, -1));
		duk_put_prop_index(ctx, -2, count++);
	}
	lua_pop(L, 2);
	return 1;
}

static duk_ret_t duk_lua_proxy_has(duk_context *ctx) {
	duk_get_prop_string(ctx, 0, "\xff" "lua");
	lua_State * L = (lua_State*)duk_get_pointer(ctx, -1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "\xff" "refrence");
	int r = duk_get_uint(ctx, -1);
	duk_pop_2(ctx);

	lua_rawgeti(L, LUA_REGISTRYINDEX, r);
	lua_getfield(L, -1, duk_safe_to_string(ctx, -1));
	duk_push_boolean(ctx, !lua_isnil(L, -1));
	lua_pop(L, 2);	
	return 1;
}

LuaDuktape::LuaDuktape(lua_State *lua) : L(lua)
{
	ctx = duk_create_heap(NULL, NULL, NULL, L, duk_lua_fatal);
	duk_require_stack(ctx, 1024);
}

LuaDuktape::~LuaDuktape()
{
	duk_destroy_heap(ctx);
}

// LuaVoxelManip()
// Creates an LuaVoxelManip and leaves it on top of stack
int LuaDuktape::create_object(lua_State *L)
{
	LuaDuktape *o = new LuaDuktape(L);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
	return 1;
}

LuaDuktape *LuaDuktape::checkobject(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		luaL_typerror(L, narg, className);

	return *(LuaDuktape **)ud;  // unbox pointer
}

int LuaDuktape::l_run(lua_State *L)
{

	LuaDuktape *o = checkobject(L, 1);
	std::string code = luaL_checkstring(L, 2);
	if ( duk_peval_lstring(o->ctx, code.data(), code.size()) == 0 ) {
		transfer(o->ctx, L, -1);
	} else {
		transfer(o->ctx, L, -1);
		lua_error(L);
	}

	return 1;
}

int LuaDuktape::l_push(lua_State *L)
{
	LuaDuktape *o = checkobject(L, 1);
	transfer(L, o->ctx, 2);
	return 0;
}

int LuaDuktape::l_pop(lua_State *L) {
	LuaDuktape *o = checkobject(L, 1);
	duk_pop(o->ctx);
	return 0;
}

int LuaDuktape::l_push_global_object(lua_State *L) {
	LuaDuktape *o = checkobject(L, 1);
	duk_push_global_object(o->ctx);
	return 0;
}

int LuaDuktape::l_put_global_string(lua_State *L) {
	LuaDuktape *o = checkobject(L, 1);
	const char * name = luaL_checkstring(L, 2);
	duk_put_global_string(o->ctx, name);
	return 0;
}

int LuaDuktape::l_get(lua_State *L)
{
	LuaDuktape *o = checkobject(L, 1);
	int idx = luaL_checkint(L, 2);
	transfer(o->ctx, L, idx);
	return 1;
}

static int fffidx = 0;
void LuaDuktape::transfer(duk_context *ctx, lua_State *L, int idx)
{
	switch ( duk_get_type(ctx, idx) ) {
		case DUK_TYPE_UNDEFINED:
			lua_pushnil(L);
			break;
		case DUK_TYPE_NUMBER:
			lua_pushnumber(L, duk_get_number(ctx, idx));
			break;
		case DUK_TYPE_BOOLEAN:
			lua_pushboolean(L, duk_get_boolean(ctx, idx));
			break;
		case DUK_TYPE_STRING:
			lua_pushstring(L, duk_to_string(ctx, idx));
			break;
		case DUK_TYPE_POINTER:
			lua_pushlightuserdata(L, duk_get_pointer(ctx, idx));
			break;
		case DUK_TYPE_OBJECT:
			 duk_push_global_stash(ctx);
			 duk_dup(ctx, idx);
			 duk_put_prop_index(ctx, -2, ++fffidx);
			 duk_pop(ctx);
			if ( duk_is_function(ctx, idx)) {
				void * lud = duk_get_heapptr(ctx, idx);
				*(void **)(lua_newuserdata(L, sizeof(void *))) = ctx;
				*(void **)(lua_newuserdata(L, sizeof(void *))) = lud;
				lua_pushcclosure(L, duk_lua_closure_call, 2);
			} else {
				//lua_newuserdata(L, 0);
				lua_newtable(L);
			}
			{	
				lua_newtable(L);
				int metatable = lua_gettop(L);
				void * lud = duk_get_heapptr(ctx, idx);

				lua_pushliteral(L, "__index");
				*(void **)(lua_newuserdata(L, sizeof(void *))) = ctx;
				*(void **)(lua_newuserdata(L, sizeof(void *))) = lud;
				lua_pushcclosure(L, duk_lua_meta_index, 2);
				lua_settable(L, metatable);

				lua_pushliteral(L, "__tostring");
				*(void **)(lua_newuserdata(L, sizeof(void *))) = ctx;
				*(void **)(lua_newuserdata(L, sizeof(void *))) = lud;
				lua_pushcclosure(L, duk_lua_meta_tostring, 2);
				lua_settable(L, metatable);

				//Create RedHerring
				lua_pushstring(L, "redherring");
				lua_newuserdata(L, 0);

				lua_newtable(L);
				lua_pushliteral(L, "__gc");
				lua_pushnumber(L, fffidx);
				*(void **)(lua_newuserdata(L, sizeof(void *))) = ctx;
				lua_pushcclosure(L, duk_lua_meta_gc, 2);
				lua_settable(L, -3);
				lua_setmetatable(L, -2);
				lua_settable(L, metatable);


				lua_setmetatable(L, -2);
			}
			break;
		default:
			const char * result = duk_safe_to_string(ctx, idx);
			lua_pushstring(L, result);
			break;
	}
}

void LuaDuktape::transfer(lua_State *L, duk_context *ctx, int idx)
{
	switch (lua_type (L, idx)) {
		case LUA_TNIL:
		case LUA_TNONE:
			duk_push_null(ctx);
			break;
		case LUA_TNUMBER:
			duk_push_number(ctx, lua_tonumber(L, idx));
			break;
		case LUA_TSTRING:
			duk_push_string(ctx, lua_tostring(L, idx));
			break;
		case LUA_TBOOLEAN:
			duk_push_boolean(ctx, lua_toboolean(L, idx));
			break;
		case LUA_TFUNCTION:
			{
				int nf = duk_push_c_function(ctx, duk_lua_call, DUK_VARARGS);
				duk_push_uint(ctx, luaL_ref(L, LUA_REGISTRYINDEX));
				duk_put_prop_string(ctx, nf, "\xff" "refrence");
				duk_push_pointer(ctx, (void *)L);
				duk_put_prop_string(ctx, nf, "\xff" "lua");
				duk_push_c_function(ctx, duk_lua_call_finalize, 1);
				duk_set_finalizer(ctx, -2);
			}
			break;
		case LUA_TUSERDATA:
		case LUA_TTABLE:
		case LUA_TLIGHTUSERDATA:
		case LUA_TTHREAD:
			{
				int nf = duk_push_object(ctx);
				duk_push_uint(ctx, luaL_ref(L, LUA_REGISTRYINDEX));
				duk_put_prop_string(ctx, nf, "\xff" "refrence");
				duk_push_pointer(ctx, (void *)L);
				duk_put_prop_string(ctx, nf, "\xff" "lua");
				duk_push_c_function(ctx, duk_lua_call_finalize, 1);
				duk_set_finalizer(ctx, -2);			
				duk_push_object(ctx);

				duk_push_c_function(ctx, duk_lua_proxy_get, 3);
				duk_put_prop_string(ctx, -2, "get");

				duk_push_c_function(ctx, duk_lua_proxy_ownkeys, 1);
				duk_put_prop_string(ctx, -2, "ownKeys");

				duk_push_c_function(ctx, duk_lua_proxy_has, 2);
				duk_put_prop_string(ctx, -2, "has");

				duk_push_proxy(ctx, 0);
			}
			break;
		default:
			printf("ERR STR (%d) %d\n", idx, lua_type (L, idx));
			lua_pushstring(L, "Cant figure the type out");
			lua_error(L);
	}
}

int LuaDuktape::l_peval(lua_State *L)
{
	LuaDuktape *o = checkobject(L, 1);
	lua_pushboolean(L, 0 == duk_peval(o->ctx));
	return 1;
}

void LuaDuktape::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);
	luaL_newmetatable(L, className);
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);  // hide metatable from Lua getmetatable()

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, gc_object);
	lua_settable(L, metatable);

	lua_pop(L, 1);  // drop metatable

	luaL_openlib(L, 0, methods, 0);  // fill methodtable
	lua_pop(L, 1);  // drop methodtable

	// Can be created from Lua (Duktape())
	lua_register(L, className, create_object);
}

const char LuaDuktape::className[] = "Duktape";
const luaL_Reg LuaDuktape::methods[] = {
	
	luamethod(LuaDuktape, run),
	luamethod(LuaDuktape, push),
	luamethod(LuaDuktape, pop),
	luamethod(LuaDuktape, get),
	luamethod(LuaDuktape, peval),

	luamethod(LuaDuktape, push_global_object),
	luamethod(LuaDuktape, put_global_string),
	{0,0}
};
