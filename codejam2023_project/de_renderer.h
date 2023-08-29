#ifndef DE_RENDERER_H
#define DE_RENDERER_H

#include <stdio.h>
#include <stdlib.h>

#include "opengl.h"

#include "external/smol/smol_utils.h"
#include "external/smol/smol_math.h"

//#define SMOL_XML_IMPLEMENTATION
#include "external/smol/smol_xml.h"

typedef struct _de_vertex_t {
	smol_v3_t position;
	smol_v3_t normal;
	smol_v2_t tex_coord;

	float bone_weights[4];
	int bone_ids[4];
} de_vertex_t;

typedef struct _de_mesh_t {
	GLuint vao, vbo, ibo;
	size_t element_count;
} de_mesh_t;

de_mesh_t de_mesh_load(smol_xml_t* xml, smol_xml_node_t* mesh);

typedef struct _de_shader_t {
	GLuint program;
} de_shader_t;

de_shader_t de_shader_create(const char* vs, const char* fs);
void de_shader_bind(de_shader_t* shader);

void de_shader_uniform1i(de_shader_t* shader, const char* name, int v);
void de_shader_uniform1f(de_shader_t* shader, const char* name, float v);
void de_shader_uniform2f(de_shader_t* shader, const char* name, smol_v2_t v);
void de_shader_uniform3f(de_shader_t* shader, const char* name, smol_v3_t v);
void de_shader_uniform4f(de_shader_t* shader, const char* name, smol_v4_t v);
void de_shader_uniform_m4f(de_shader_t* shader, const char* name, smol_m4_t v);

typedef struct _de_texture_t {
	GLuint id;
	GLuint width, height;
} de_texture_t;

de_texture_t de_texture_create(const void* rgba_data, GLuint width, GLuint height);

typedef void (*de_shader_setup_cb)(de_shader_t*, void*);

#define DE_DRAWABLE_TEXTURES_COUNT 3
typedef struct _de_drawable_t {
	de_mesh_t* mesh;
	de_shader_t* shader;
	de_texture_t* textures[DE_DRAWABLE_TEXTURES_COUNT];

	GLenum blend_dfactor, blend_sfactor;
	GLenum cull_face;
	int depth_testing;

	smol_m4_t model;

	void* user_data;
	de_shader_setup_cb shader_setup;

	size_t identifier; // the system uses this to sort the drawables and improve performance
} de_drawable_t;

typedef smol_vector(de_drawable_t) de_drawable_array_t;

typedef struct _de_renderer_t {
	de_drawable_array_t drawables;
	size_t num_bindings;
	size_t num_draw_calls;

	smol_m4_t projection;
	smol_m4_t view;
} de_renderer_t;

de_renderer_t de_renderer_create();
void de_renderer_destroy(de_renderer_t* renderer);
void de_renderer_submit(de_renderer_t* renderer, de_drawable_t drawable);
void de_renderer_flush(de_renderer_t* renderer);

#endif // DE_RENDERER_H
