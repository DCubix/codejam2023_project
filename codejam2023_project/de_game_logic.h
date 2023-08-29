#ifndef DE_GAME_LOGIC_H
#define DE_GAME_LOGIC_H

#include "external/smol/smol_utils.h"
#include "external/smol/smol_math.h"
#include "de_renderer.h"

#define DE_INVALID_OBJECT (size_t)(-1)

typedef struct _de_object_t de_object_t;
typedef struct _de_scene_t de_scene_t;

typedef void(*de_object_behavior_cb)(de_scene_t*, de_object_t*, float);

typedef struct _de_object_ctor_t {
	smol_v3_t position;
	smol_v3_t scale;
	smol_quat_t rotation;

	float life;

	size_t object_id;
} de_object_ctor_t;

typedef smol_vector(de_object_ctor_t) de_object_ctor_array_t;

typedef void(*de_component_free_cb)(void*);
typedef void(*de_component_create_cb)(de_scene_t*, de_object_t*, void*);
typedef void(*de_component_update_cb)(de_scene_t*, de_object_t*, void*, float);
typedef void(*de_component_render_cb)(de_scene_t*, de_renderer_t*, de_object_t*, void*);
typedef void(*de_component_destroy_cb)(de_scene_t*, de_object_t*, void*);

typedef enum _de_builtin_component_type {
	DE_BUILTIN_COMPONENT_MESH = 0,
	DE_BUILTIN_COMPONENT_CAMERA,
	DE_BUILTIN_COMPONENT_LIGHT,
	DE_BUILTIN_COMPONENT_SCRIPT
} de_builtin_component_type;

typedef struct _de_mesh_component_t {
	de_mesh_t* mesh;
	de_shader_t* shader;
	de_texture_t* textures[DE_DRAWABLE_TEXTURES_COUNT];
} de_mesh_component_t;

typedef struct _de_camera_component_t {
	float fov, z_near, z_far, aspect;
} de_camera_component_t;

typedef struct _de_component_t {
	size_t type;
	de_component_free_cb free_func;
	de_component_create_cb create_func;
	de_component_update_cb update_func;
	de_component_render_cb render_func;
	de_component_destroy_cb destroy_func;
	void* data;
} de_component_t;

void de_mesh_component_render_func(de_scene_t* scene, de_renderer_t* renderer, de_object_t* object, void* mesh_comp);
void de_camera_component_render_func(de_scene_t* scene, de_renderer_t* renderer, de_object_t* object, void* comp);
void de_script_component_create_func(de_scene_t* scene, de_object_t* object, void* comp);
void de_script_component_update_func(de_scene_t* scene, de_object_t* object, void* comp, float delta);
void de_script_component_render_func(de_scene_t* scene, de_renderer_t* renderer, de_object_t* object, void* comp);
void de_script_component_destroy_func(de_scene_t* scene, de_object_t* object, void* comp);

typedef struct _de_prop_t {
	char name[128];

	enum {
		DE_PROP_STRING = 0,
		DE_PROP_INT,
		DE_PROP_FLOAT,
		DE_PROP_BOOL
	} type;

	union {
		int int_prop;
		float flt_prop;
		char str_prop[128];
	};
} de_prop_t;

typedef smol_vector(de_component_t) de_component_array_t;
typedef smol_vector(de_prop_t) de_prop_array_t;

typedef struct _de_object_t {
	char tag[32];
	char name[128];

	size_t id;
	size_t parent;

	smol_v3_t position;
	smol_v3_t scale;
	smol_quat_t rotation;

	de_component_array_t components;
	de_prop_array_t properties;

	float life;
	int is_dead;
	int is_created;
} de_object_t;

void de_object_init(de_object_t* object);
void de_object_free(de_object_t* object);
smol_m4_t de_object_get_matrix(de_object_t* object);
void de_object_add_component(
	de_object_t* object, size_t type, void* component,
	de_component_render_cb render_func,
	de_component_update_cb update_func
);
void* de_object_get_component(de_object_t* object, size_t type);

de_prop_t* de_object_get_prop(de_object_t* object, const char* name);
void de_object_set_prop_string(de_object_t* object, const char* name, const char* value);
void de_object_set_prop_float(de_object_t* object, const char* name, float value);
void de_object_set_prop_int(de_object_t* object, const char* name, int value);

typedef smol_vector(de_object_t) de_object_array_t;
typedef smol_vector(de_object_t*) de_objectptr_array_t;

typedef struct _de_scene_t {
	de_object_array_t objects;
	de_object_ctor_array_t object_ctors;

	de_object_t* camera;
} de_scene_t;

void de_scene_init(de_scene_t* scene);
void de_scene_free(de_scene_t* scene);

de_object_t* de_scene_get_object(de_scene_t* scene, size_t id);
de_object_t* de_scene_get_object_by_name(de_scene_t* scene, const char* name);
void de_scene_get_objects_by_tag(de_scene_t* scene, size_t tag, de_objectptr_array_t* out);

de_object_t* de_scene_get_object_by_component(de_scene_t* scene, size_t type);
void de_scene_get_objects_by_component(de_scene_t* scene, size_t type, de_objectptr_array_t* out);

smol_m4_t de_scene_compute_object_matrix(de_scene_t* scene, size_t id);

void de_scene_update(de_scene_t* scene, float delta_time);
void de_scene_render(de_scene_t* scene, de_renderer_t* renderer);

size_t de_scene_spawn(de_scene_t* scene, de_object_ctor_t ctor);
void de_scene_object_parent(de_scene_t* scene, size_t id, size_t parent);

extern size_t _de_scene_global_id;

#endif // DE_GAME_LOGIC_H
