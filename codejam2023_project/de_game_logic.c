#include "de_game_logic.h"

//#define SMOL_XML_IMPLEMENTATION
#include "external/smol/smol_xml.h"

#include "de_asset_manager.h"

size_t _de_scene_global_id = 0;

#define _swap_neg(a, b) do { \
	float _tmp = a; \
	a = -b; \
	b = _tmp; \
} while(0);

void de_mesh_component_render_func(de_scene_t* scene, de_renderer_t* renderer, de_object_t* object, void* mesh_comp) {
	de_mesh_component_t* comp = (de_mesh_component_t*)mesh_comp;
	smol_m4_t mat = de_scene_compute_object_matrix(scene, object->id);

	de_drawable_t drw = { 0 };
	drw.identifier = object->id;
	drw.model = mat;
	drw.shader = comp->shader;
	drw.mesh = comp->mesh;
	drw.blend_sfactor = GL_SRC_ALPHA;
	drw.blend_dfactor = GL_ONE_MINUS_SRC_ALPHA;

	for (size_t i = 0; i < DE_DRAWABLE_TEXTURES_COUNT; i++)
		drw.textures[i] = comp->textures[i];

	de_renderer_submit(renderer, drw);
}

void de_camera_component_render_func(de_scene_t* scene, de_renderer_t* renderer, de_object_t* object, void* comp) {
	scene->camera = object;
}

void de_script_component_create_func(de_scene_t* scene, de_object_t* object, void* comp) {
	de_script_t* script = (de_script_t*)comp;
	if (script->on_create) {
		de_scripting_call_no_args(script->L, script->on_create, object);
	}
}

void de_script_component_update_func(de_scene_t* scene, de_object_t* object, void* comp, float delta) {
	de_script_t* script = (de_script_t*)comp;
	if (script->on_update) {
		de_scripting_call_float_arg(script->L, script->on_update, delta, object);
	}
}

void de_script_component_render_func(de_scene_t* scene, de_renderer_t* renderer, de_object_t* object, void* comp) {
	de_script_t* script = (de_script_t*)comp;
	if (script->on_draw) {
		de_scripting_call_no_args(script->L, script->on_draw, object);
	}
}

void de_script_component_destroy_func(de_scene_t* scene, de_object_t* object, void* comp) {
	de_script_t* script = (de_script_t*)comp;
	if (script->on_destroy) {
		de_scripting_call_no_args(script->L, script->on_destroy, object);
	}
}

void de_object_init(de_object_t* object) {
	smol_vector_init(&object->components, 16);
	smol_vector_init(&object->properties, 16);
}

void de_object_free(de_object_t* object) {
	smol_vector_each(&object->components, de_component_t, comp) {
		if (comp->free_func && comp->data) comp->free_func(comp->data);
	}
	smol_vector_free(&object->components);

	smol_vector_each(&object->properties, de_prop_t, prop) {
		if (prop->type == DE_PROP_STRING && prop->str_prop) free(prop->str_prop);
	}
	smol_vector_free(&object->properties);

	object->is_dead = 1;
}

smol_m4_t de_object_get_matrix(de_object_t* object) {
	smol_m4_t tr = smol_m4_translate_vec(object->position);
	smol_m4_t rt = smol_m4_rotate_quat(object->rotation);
	smol_m4_t sc = smol_m4_scale_vec(object->scale);
	return smol_m4_mul(tr, smol_m4_mul(rt, sc));
}

void de_object_add_component(
	de_object_t* object, size_t type, void* component,
	de_component_render_cb render_func,
	de_component_update_cb update_func
) {
	de_component_t comp = { 0 };
	comp.free_func = free;
	comp.data = component;
	comp.type = type;
	comp.render_func = render_func;
	comp.update_func = update_func;
	smol_vector_push(&object->components, comp);
}

void* de_object_get_component(de_object_t* object, size_t type) {
	smol_vector_each(&object->components, de_component_t, comp) {
		if (comp->type == type) return comp->data;
	}
	return NULL;
}

de_prop_t* de_object_get_prop(de_object_t* object, const char* name) {
	smol_vector_each(&object->properties, de_prop_t, prop) {
		if (strcmp(prop->name, name) == 0) return prop;
	}
	return NULL;
}

void de_object_set_prop_string(de_object_t* object, const char* name, const char* value) {
	de_prop_t* prop = de_object_get_prop(object, name);
	SMOL_ASSERT(prop != NULL);
	SMOL_ASSERT(prop->type == DE_PROP_STRING);
	strcpy(prop->str_prop, value);
}

void de_object_set_prop_float(de_object_t* object, const char* name, float value) {
	de_prop_t* prop = de_object_get_prop(object, name);
	SMOL_ASSERT(prop != NULL);
	SMOL_ASSERT(prop->type == DE_PROP_FLOAT);
	prop->flt_prop = value;
}

void de_object_set_prop_int(de_object_t* object, const char* name, int value) {
	de_prop_t* prop = de_object_get_prop(object, name);
	SMOL_ASSERT(prop != NULL);
	SMOL_ASSERT(prop->type == DE_PROP_INT || prop->type == DE_PROP_BOOL);
	prop->int_prop = value;
}

void de_scene_init(de_scene_t* scene) {
	SMOL_ASSERT(scene != NULL);
	smol_vector_init(&scene->objects, 2048);
	smol_vector_init(&scene->object_ctors, 128);
	scene->camera = NULL;
}

void de_scene_free(de_scene_t* scene) {
	SMOL_ASSERT(scene != NULL);

	smol_vector_each(&scene->objects, de_object_t, obj) {
		de_object_free(obj);
	}

	smol_vector_free(&scene->objects);
	smol_vector_free(&scene->object_ctors);
}

de_object_t* de_scene_get_object(de_scene_t* scene, size_t id) {
	if (id == DE_INVALID_OBJECT) return NULL;
	if (id >= smol_vector_count(&scene->objects)) return NULL;
	smol_vector_each(&scene->objects, de_object_t, obj) {
		if (obj->id == id) return obj;
	}
	return NULL;
}

de_object_t* de_scene_get_object_by_name(de_scene_t* scene, const char* name) {
	SMOL_ASSERT(name != NULL);
	smol_vector_each(&scene->objects, de_object_t, obj) {
		if (strcmp(obj->name, name) == 0) return obj;
	}
	return NULL;
}

void de_scene_get_objects_by_tag(de_scene_t* scene, size_t tag, de_objectptr_array_t* out) {
	smol_vector_each(&scene->objects, de_object_t, obj) {
		if (obj->tag == tag) smol_vector_push(out, obj);
	}
}

de_object_t* de_scene_get_object_by_component(de_scene_t* scene, size_t type) {
	smol_vector_each(&scene->objects, de_object_t, obj) {
		smol_vector_each(&obj->components, de_component_t, comp) {
			if (comp->type == type) return obj;
		}
	}
	return NULL;
}

void de_scene_get_objects_by_component(de_scene_t* scene, size_t type, de_objectptr_array_t* out) {
	smol_vector_each(&scene->objects, de_object_t, obj) {
		smol_vector_each(&obj->components, de_component_t, comp) {
			if (comp->type == type) smol_vector_push(out, obj);
		}
	}
}

smol_m4_t de_scene_compute_object_matrix(de_scene_t* scene, size_t id) {
	de_object_t* obj = de_scene_get_object(scene, id);
	SMOL_ASSERT(obj != NULL);

	smol_m4_t parent_mat = smol_m4_identity();
	if (obj->parent != DE_INVALID_OBJECT) {
		parent_mat = de_scene_compute_object_matrix(scene, obj->parent);
	}

	return smol_m4_mul(parent_mat, de_object_get_matrix(obj));
}

size_t _de_scene_index_of(de_scene_t* scene, size_t obj_id) {
	size_t i = 0;
	smol_vector_each(&scene->objects, de_object_t, obj) {
		if (obj->id == obj_id) {
			return i;
		}
		i++;
	}
	i = DE_INVALID_OBJECT;
	return i;
}

void de_scene_update(de_scene_t* scene, float delta_time) {
	smol_vector(size_t) dead;
	smol_vector_init(&dead, 128);

	smol_vector_each(&scene->objects, de_object_t, obj) {
		// quick hack to determine prefabs, which are templates for object creation.
		// if the tag starts with a $ then it's a prefab :P
		if (obj->tag[0] == '$') continue;

		if (!obj->is_created) {
			smol_vector_each(&obj->components, de_component_t, comp) {
				if (comp->create_func) comp->create_func(scene, obj, comp->data);
			}
			obj->is_created = 1;
		}

		if (obj->life > 0.0f) {
			obj->life -= delta_time;
			if (obj->life <= 0.0f) {
				obj->life = 0.0f;
				obj->is_dead = 1;
			}
		}

		if (obj->is_dead) {
			smol_vector_each(&obj->components, de_component_t, comp) {
				if (comp->destroy_func) comp->destroy_func(scene, obj, comp->data);
			}
			smol_vector_push(&dead, obj->id);
			continue;
		}

		smol_vector_each(&obj->components, de_component_t, comp) {
			if (comp->update_func) comp->update_func(scene, obj, comp->data, delta_time);
		}
	}

	smol_vector_each(&dead, size_t, id) {
		smol_vector_remove(&scene->objects, _de_scene_index_of(scene, *id));
	}

	smol_vector_free(&dead);
}

void de_scene_render(de_scene_t* scene, de_renderer_t* renderer) {
camera_setup:
	if (scene->camera) {
		de_camera_component_t* camera = (de_camera_component_t*)de_object_get_component(
			scene->camera, DE_BUILTIN_COMPONENT_CAMERA
		);

		renderer->projection = smol_m4_perspective_lh(
			camera->fov,
			camera->aspect,
			camera->z_near,
			camera->z_far
		);

		smol_m4_t cam_pos = smol_m4_translate_vec(smol_v3_neg(scene->camera->position));
		smol_m4_t cam_rot = smol_m4_rotate_quat(smol_quat_conjugate(scene->camera->rotation));
		smol_m4_t view = smol_m4_mul(cam_rot, cam_pos);
		renderer->view = view;
	}
	else {
		scene->camera = de_scene_get_object_by_component(scene, DE_BUILTIN_COMPONENT_CAMERA);
		goto camera_setup;
	}

	smol_vector_each(&scene->objects, de_object_t, obj) {
		// quick hack to determine prefabs, which are templates for object creation.
		// if the tag starts with a $ then it's a prefab :P
		if (obj->tag[0] == '$') continue;

		if (obj->is_dead) {
			continue;
		}
		
		smol_vector_each(&obj->components, de_component_t, comp) {
			if (comp->render_func) comp->render_func(scene, renderer, obj, comp->data);
		}
	}
}

size_t de_scene_spawn(de_scene_t* scene, de_object_ctor_t ctor) {
	ctor.object_id = _de_scene_global_id++;
	smol_vector_push(&scene->object_ctors, ctor);
	return ctor.object_id;
}

void de_scene_object_parent(de_scene_t* scene, size_t id, size_t parent) {
	de_object_t* obj = de_scene_get_object(scene, id);
	SMOL_ASSERT(obj != NULL);
	SMOL_ASSERT(de_scene_get_object(scene, parent) != NULL);
	obj->parent = parent;
}
