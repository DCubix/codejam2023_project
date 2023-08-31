#include "de_scripting.h"

#include "de_asset_manager.h"
#include "de_game_logic.h"
#include "external/smol/smol_xml.h"

void writeFn(WrenVM* vm, const char* text) {
	printf("%s", text);
}

void errorFn(WrenVM* vm, WrenErrorType errorType,
	const char* module, const int line,
	const char* msg
) {
	switch (errorType) 	{
		case WREN_ERROR_COMPILE: {
			printf("[%s line %d] [Error] %s\n", module, line, msg);
		} break;
		case WREN_ERROR_STACK_TRACE: {
			printf("[%s line %d] in %s\n", module, line, msg);
		} break;
		case WREN_ERROR_RUNTIME: {
			printf("[Runtime Error] %s\n", msg);
		} break;
	}
}

void de_scripting_call_no_args(WrenVM* vm, WrenHandle* ref, WrenHandle* receiver) {
	wrenEnsureSlots(vm, 1);
	wrenSetSlotHandle(vm, 0, receiver);
	wrenCall(vm, ref);
}

void de_scripting_call_float_arg(WrenVM* vm, WrenHandle* ref, float arg, WrenHandle* receiver) {
	wrenEnsureSlots(vm, 3);
	wrenSetSlotHandle(vm, 0, receiver);
	wrenSetSlotDouble(vm, 2, (double)arg);
	wrenCall(vm, ref);
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

int _test_ident(char c) { return isalnum(c) || c == '_'; }

de_script_t* de_scripting_run_script(WrenVM* vm, const char* file_name) {
	size_t size;
	const char* src = (char*)smol_read_entire_file(file_name, &size);

	char* mod;

	// extract module name from the first class in the script
	{
		smol_scanner_t sc = { 0 };
		smol_scanner_init(&sc, src);

		while (!smol_scanner_has_ahead(&sc, "class")) {
			smol_scanner_skip(&sc);
		}
		smol_scanner_skip(&sc);
		smol_scanner_skip(&sc);
		smol_scanner_skip(&sc);
		smol_scanner_skip(&sc);
		smol_scanner_skip(&sc);
		smol_scanner_skip_spaces(&sc);

		mod = smol_scanner_read(&sc, _test_ident);
	}

	wrenInterpret(vm, "main", src);

	free(src);

	de_script_t script = { 0 };
	wrenEnsureSlots(vm, 2);
	wrenGetVariable(vm, "main", mod, 0);
	wrenGetVariable(vm, "main", "de_object", 1);

	script.receiver = wrenGetSlotHandle(vm, 0);
	script.object_class = wrenGetSlotHandle(vm, 1);
	script.on_create = wrenMakeCallHandle(vm, "on_create(_)");
	script.on_update = wrenMakeCallHandle(vm, "on_update(_,_)");
	script.on_destroy = wrenMakeCallHandle(vm, "on_destroy(_)");
	script.vm = vm;

	de_assets_load_script(mod, script);
	return de_assets_get_script(mod);
}

#include "de_binding.h"

WrenVM* de_scripting_init() {
	WrenConfiguration config;
	wrenInitConfiguration(&config);

	config.writeFn = &writeFn;
	config.errorFn = &errorFn;
	config.bindForeignMethodFn = &bindForeignMethod;
	config.bindForeignClassFn = &bindForeignClass;

	WrenVM* vm = wrenNewVM(&config);

	// gamelib
	const char* gamelib =
#include "wren_gamelib.h"
		;
	wrenInterpret(vm, "main", gamelib);

//	const char* gamelib_test =
//#include "wren_gamelib_test.h"
//		;
//	wrenInterpret(vm, "main", gamelib_test);

	return vm;
}

void de_scripting_destroy(WrenVM* vm) {
	wrenFreeVM(vm);
}
