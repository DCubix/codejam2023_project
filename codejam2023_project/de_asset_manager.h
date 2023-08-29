#ifndef DE_ASSET_MANAGER_H
#define DE_ASSET_MANAGER_H

#include <stdint.h>

#include "de_renderer.h"
#include "de_scripting.h"
#include "external/smol/smol_xml.h"

typedef struct _de_scene_t de_scene_t;
typedef struct _de_game_t de_game_t;

typedef smol_vector(de_mesh_t) de_mesh_array_t;
typedef smol_vector(de_shader_t) de_shader_array_t;
typedef smol_vector(de_texture_t) de_texture_array_t;
typedef smol_vector(de_script_t) de_script_array_t;
typedef smol_vector(de_scene_t) de_scene_array_t;
typedef smol_vector(uint32_t) de_index_array_t;

typedef struct _de_asset_manager_t {
	de_mesh_array_t meshes;
	de_texture_array_t textures;
	de_shader_array_t shaders;
	de_script_array_t scripts;
	de_scene_array_t scenes;

	de_index_array_t mesh_hashes;
	de_index_array_t texture_hashes;
	de_index_array_t shader_hashes;
	de_index_array_t script_hashes;
	de_index_array_t scene_hashes;
} de_asset_manager_t;

extern de_asset_manager_t de_assets;

void de_assets_load_mesh(const char* name, smol_xml_t* xml, smol_xml_node_t* node);
void de_assets_load_texture(const char* name, const char* file_name);
void de_assets_load_shader(const char* name, const char* vs_file_name, const char* fs_file_name);
void de_assets_load_script(const char* name, de_script_t script);
void de_assets_load_scene(de_game_t* game, const char* name, const char* file_path);

void de_asset_manager_init();
void de_asset_manager_free();

de_mesh_t* de_assets_get_mesh(const char* name);
de_texture_t* de_assets_get_texture(const char* name);
de_shader_t* de_assets_get_shader(const char* name);
de_script_t* de_assets_get_script(const char* name);
de_scene_t* de_assets_get_scene(const char* name);

int de_assets_has_mesh(const char* name);
int de_assets_has_texture(const char* name);
int de_assets_has_shader(const char* name);
int de_assets_has_script(const char* name);
int de_assets_has_scene(const char* name);

#endif // DE_ASSET_MANAGER_H
