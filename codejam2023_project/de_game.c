#include "de_game.h"

#pragma region Debug
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length,
    const GLchar* msg, const void* data)
{
    char* _source;
    char* _type;
    char* _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API_ARB:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION_ARB:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER_ARB:
            _source = "UNKNOWN";
            break;

        default:
            _source = "UNKNOWN";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR_ARB:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            _type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY_ARB:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER_ARB:
            _type = "OTHER";
            break;

        default:
            _type = "UNKNOWN";
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH_ARB:
            _severity = "HIGH";
            break;

        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            _severity = "MEDIUM";
            break;

        case GL_DEBUG_SEVERITY_LOW_ARB:
            _severity = "LOW";
            break;

        default:
            _severity = "UNKNOWN";
            break;
    }

    printf("%d: %s of %s severity, raised from %s: %s\n",
        id, _type, _severity, _source, msg);
}
#pragma endregion

void de_game_start(de_game_t* game) {
    smol_frame_config_t conf = { 0 };
    smol_frame_gl_spec_t glconf = { 0 };
    glconf.alpha_bits = 8;
    glconf.red_bits = 8;
    glconf.green_bits = 8;
    glconf.blue_bits = 8;
    glconf.stencil_bits = 8;
    glconf.depth_bits = 16;
    glconf.is_debug = 1;
    glconf.minor_version = 3;
    glconf.major_version = 3;

    conf.width = 1280;
    conf.height = 720;
    conf.title = "CodeJam 2023";
    conf.gl_spec = &glconf;

    smol_frame_t* frame = smol_frame_create_advanced(&conf);

#ifndef __EMSCRIPTEN__
    gladLoadGL();
#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallbackARB(GLDebugMessageCallback, NULL);

    game->L = de_scripting_init();
    game->renderer = de_renderer_create();

    de_asset_manager_init();
    de_assets_load_texture("level_map", "level_map.qoi");
    de_assets_load_shader("default", "basic.vert", "basic.frag");

    de_assets_load_scene(game, "level", "level_data.xml");

    game->scene = NULL;
    game->next_scene = de_assets_get_scene("level");

    double start_time = smol_timer();
    double accum = 0.0;

    while (!smol_frame_is_closed(frame)) {
        int do_render = 0;
        double current_time = smol_timer();
        double delta = current_time - start_time;
        start_time = current_time;

        accum += delta;

        smol_frame_update(frame);

        SMOL_FRAME_EVENT_LOOP(frame, ev) {

        }

        while (accum >= DE_GAME_TIME_STEP) {
            if (game->scene == NULL) {
                game->scene = game->next_scene;
            }

            if (game->scene) de_scene_update(game->scene, (float)DE_GAME_TIME_STEP);

            accum -= DE_GAME_TIME_STEP;
            do_render = 1;
        }

        if (do_render) {
            glClearColor(0.0f, 0.1f, 0.4f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            if (game->scene) {
                de_scene_render(game->scene, &game->renderer);
            }

            de_renderer_flush(&game->renderer);
            smol_frame_gl_swap_buffers(frame);
        }
    }

    de_renderer_destroy(&game->renderer);
    de_scripting_destroy(game->L);
    de_asset_manager_free();
    smol_frame_destroy(frame);
}
