#ifndef DE_SCRIPTING_H
#define DE_SCRIPTING_H

#include "external/wren.h"

#define DE_SCRIPT_CREATE_FN on_create
#define DE_SCRIPT_UPDATE_FN on_update
#define DE_SCRIPT_DESTROY_FN on_destroy

typedef struct _de_script_t de_script_t;
typedef struct _de_object_t de_object_t;

typedef struct _de_script_t {
	WrenVM* vm;

	WrenHandle* on_create;
	WrenHandle* on_update;
	WrenHandle* on_destroy;

	WrenHandle* receiver;

	WrenHandle* object_class;
} de_script_t;

WrenVM* de_scripting_init();
void de_scripting_destroy(WrenVM* vm);

void de_scripting_call_no_args(WrenVM* vm, WrenHandle* ref, WrenHandle* receiver);
void de_scripting_call_float_arg(WrenVM* vm, WrenHandle* ref, float arg, WrenHandle* receiver);

de_script_t* de_scripting_run_script(WrenVM* vm, const char* file_name);

void de__reg_push_object_metatable(WrenVM* vm, de_object_t* self);

#endif // DE_SCRIPTING_H
