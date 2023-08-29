#include "de_scripting.h"

#include "de_asset_manager.h"
#include "de_game_logic.h"

#define luaL_checkboolean(L,n) (luaL_checktype(L, (n), LUA_TBOOLEAN))
#define luaL_checkfunction(L,n) (luaL_checktype(L, (n), LUA_TFUNCTION))

void dumpstack(lua_State* L) {
	printf("=========================================\n");
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++) {
		printf("%d\t%s\t", i, luaL_typename(L, i));
		switch (lua_type(L, i)) {
			case LUA_TNUMBER:
				printf("%g\n", lua_tonumber(L, i));
				break;
			case LUA_TSTRING:
				printf("%s\n", lua_tostring(L, i));
				break;
			case LUA_TBOOLEAN:
				printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
				break;
			case LUA_TNIL:
				printf("%s\n", "nil");
				break;
			default:
				printf("%p\n", lua_topointer(L, i));
				break;
		}
	}
}

#ifndef _NDEBUG
#define dump() dumpstack(L)
#else
#define dump()
#endif

void de_scripting_call_no_args(lua_State* L, int ref, de_object_t* self) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	if (lua_isfunction(L, -1)) {
		de__reg_push_object_metatable(L, self);
		lua_pcall(L, 1, 0, 0);
	}
	else {
		lua_pop(L, -1);
	}
}

void de_scripting_call_float_arg(lua_State* L, int ref, float arg, de_object_t* self) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
	if (lua_isfunction(L, -1)) {
		de__reg_push_object_metatable(L, self);
		lua_pushnumber(L, arg);
		lua_pcall(L, 2, 0, 0);
	}
	else {
		lua_pop(L, -1);
	}
}

de_lua_fn de_get_function(lua_State* L, const char* name) {
	lua_getfield(L, -1, name);
	return luaL_ref(L, LUA_REGISTRYINDEX);
}

char* _clean_path(char* path) {
	size_t slash_index = 0;
	size_t i = 0;
	while (path[i] != '\0') {
		if (path[i] == '\\' || path[i] == '/') {
			slash_index = i;
		}
		i++;
	}
	path += slash_index+1;

	i = strlen(path);
	while (path[i] != '.') {
		path[i] = NULL;
		i--;
	}
	path[i] = NULL;

	return path;
}

de_script_t* de_scripting_run_script(lua_State* L, const char* file_name) {
	if (luaL_dofile(L, file_name) != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, -1);
		return NULL;
	}
	
	de_script_t script = { 0 };
	script.on_create = de_get_function(L, DE_SCRIPT_CREATE_FN);
	script.on_update = de_get_function(L, DE_SCRIPT_UPDATE_FN);
	script.on_draw = de_get_function(L, DE_SCRIPT_DRAW_FN);
	script.on_destroy = de_get_function(L, DE_SCRIPT_DESTROY_FN);
	script.L = L;
	
	lua_pop(L, -1);

	file_name = _clean_path(file_name);

	de_assets_load_script(file_name, script);
	return de_assets_get_script(file_name);
}

#define _BIND(name) static int deLua_##name(lua_State* L)
#define _ENTRY(name) { #name, deLua_##name }
#define _GUARD { NULL, NULL }

_BIND(obj_get_prop) {
	de_object_t* object = NULL;
	char* name = NULL;

	if (lua_gettop(L) < 2) {
		lua_pushnil(L);
		return 1;
	}

	object = (de_object_t*)lua_touserdata(L, 1);
	if (!object) {
		lua_pushstring(L, "Object not found.");
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}

	name = luaL_checkstring(L, 2);

	de_prop_t* prop = de_object_get_prop(object, name);
	if (!prop) {
		lua_pushfstring(L, "Object has no property named \"%s\".", name);
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}
	
	switch (prop->type) {
		case DE_PROP_STRING: lua_pushstring(L, prop->str_prop); break;
		case DE_PROP_FLOAT: lua_pushnumber(L, prop->flt_prop); break;
		case DE_PROP_INT: lua_pushinteger(L, prop->int_prop); break;
		case DE_PROP_BOOL: lua_pushboolean(L, prop->int_prop > 0 ? 1 : 0); break;
		default: lua_pushnil(L); break;
	}

	return 1;
}

_BIND(obj_set_prop) {
	de_object_t* object = NULL;
	char* name = NULL;

	if (lua_gettop(L) < 2) {
		return 0;
	}

	object = (de_object_t*)lua_touserdata(L, 1);
	if (!object) {
		lua_pushstring(L, "Object not found.");
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}

	name = luaL_checkstring(L, 2);
	if (!de_object_get_prop(object, name)) {
		lua_pushfstring(L, "Object has no property named \"%s\".", name);
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}

	if (lua_type(L, 3) == LUA_TSTRING) {
		const char* value = lua_tostring(L, 3);
		de_object_set_prop_string(object, name, value);
	}
	else if (lua_type(L, 3) == LUA_TNUMBER) {
		double value = lua_tonumber(L, 3);
		if (value == (int)value) {
			de_object_set_prop_int(object, name, (int)value);
		}
		else {
			de_object_set_prop_float(object, name, (float)value);
		}
	}
	else if (lua_type(L, 3) == LUA_TBOOLEAN) {
		int value = lua_toboolean(L, 3);
		de_object_set_prop_int(object, name, value);
	}
	else {
		lua_pushstring(L, "Invalid property type.");
		lua_error(L);
	}

	return 0;
}

_BIND(obj_get_all_props) {
	de_object_t* object = (de_object_t*)lua_touserdata(L, -1);
	if (!object) {
		lua_pushstring(L, "Object not found.");
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);

	smol_vector_each(&object->properties, de_prop_t, prop) {
		lua_pushstring(L, prop->name);
		switch (prop->type) {
			case DE_PROP_STRING: lua_pushstring(L, prop->str_prop); break;
			case DE_PROP_INT: lua_pushnumber(L, prop->int_prop); break;
			case DE_PROP_FLOAT: lua_pushnumber(L, prop->flt_prop); break;
			case DE_PROP_BOOL: lua_pushboolean(L, prop->int_prop > 0 ? 1 : 0); break;
		}
		lua_settable(L, -3);
	}

	return 1;
}

_BIND(obj_get_tag) {
	de_object_t* object = (de_object_t*)lua_touserdata(L, -1);
	if (!object) {
		lua_pushstring(L, "Object not found.");
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}
	lua_pushstring(L, object->tag);
	return 1;
}

_BIND(obj_get_name) {
	de_object_t* object = (de_object_t*)lua_touserdata(L, -1);
	if (!object) {
		lua_pushstring(L, "Object not found.");
		lua_error(L);
		lua_pushnil(L);
		return 1;
	}
	lua_pushstring(L, object->name);
	return 1;
}

lua_State* de_scripting_init() {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	return L;
}

void de_scripting_destroy(lua_State* L) {
	lua_close(L);
}

_BIND(obj_index) {
	const char* key = luaL_checkstring(L, 2);
	lua_pop(L, 1);

	if (strcmp(key, "tag") == 0) {
		return deLua_obj_get_tag(L);
	}
	else if (strcmp(key, "name") == 0) {
		return deLua_obj_get_name(L);
	}
	else if (strcmp(key, "properties") == 0) {
		return deLua_obj_get_all_props(L);
	}
	else {
		lua_pushstring(L, key);
		return deLua_obj_get_prop(L);
	}
	return 0;
}

_BIND(obj_newindex) {
	const char* key = luaL_checkstring(L, 2);

	if (strcmp(key, "tag") == 0) {
		return 0;
	}
	else if (strcmp(key, "name") == 0) {
		return 0;
	}
	else if (strcmp(key, "properties") == 0) {
		return 0;
	}
	else {
		return deLua_obj_set_prop(L);
	}

	return 0;
}

void de__reg_push_object_metatable(lua_State* L, de_object_t* object) {
	const luaL_Reg obj_funs[] = {
		_ENTRY(obj_get_prop),
		_ENTRY(obj_set_prop),
		_GUARD
	};

	lua_pushlightuserdata(L, object);

	if (luaL_newmetatable(L, "_de_object")) {
		luaL_setfuncs(L, obj_funs, 0);

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, deLua_obj_index);
		lua_settable(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, deLua_obj_newindex);
		lua_settable(L, -3);
	}

	lua_setmetatable(L, -2);
}
