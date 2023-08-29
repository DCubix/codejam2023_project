#ifndef DE_SCRIPTING_H
#define DE_SCRIPTING_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define DE_SCRIPT_CREATE_FN "on_create"
#define DE_SCRIPT_UPDATE_FN "on_update"
#define DE_SCRIPT_DRAW_FN "on_draw"
#define DE_SCRIPT_DESTROY_FN "on_destroy"

typedef int de_lua_fn;

typedef struct _de_script_t de_script_t;
typedef struct _de_object_t de_object_t;

typedef struct _de_script_t {
	lua_State* L;

	de_lua_fn on_create;
	de_lua_fn on_update;
	de_lua_fn on_draw;
	de_lua_fn on_destroy;
} de_script_t;

lua_State* de_scripting_init();
void de_scripting_destroy(lua_State* L);

void de_scripting_call_no_args(lua_State* L, int ref, de_object_t* self);
void de_scripting_call_float_arg(lua_State* L, int ref, float arg, de_object_t* self);

de_script_t* de_scripting_run_script(lua_State* L, const char* file_name);

void de__reg_push_object_metatable(lua_State* L, de_object_t* self);

#endif // DE_SCRIPTING_H
