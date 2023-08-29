#ifndef DE_GAME_H
#define DE_GAME_H

#include "external/smol/smol_canvas.h"
#include "external/smol/smol_frame.h"
#include "external/smol/smol_utils.h"

#include "opengl.h"
#include "de_renderer.h"
#include "de_game_logic.h"
#include "de_asset_manager.h"
#include "de_scripting.h"

#define DE_GAME_TIME_STEP (1.0 / 60.0)

typedef struct _de_game_t {
	de_scene_t* scene;
	de_scene_t* next_scene;

	de_renderer_t renderer;

	enum {
		DE_GAME_STATE_INVALID = 0,
		DE_GAME_STATE_LOADING,
		DE_GAME_STATE_GAMEPLAY
	} game_state;

	lua_State* L;
} de_game_t;

void de_game_start(de_game_t* game);

#endif // DE_GAME_H
