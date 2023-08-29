#include "de_renderer.h"

#include "external/smol/smol_canvas.h"

typedef smol_vector(smol_v3_t) de_v3_list_t;
typedef smol_vector(smol_v2_t) de_v2_list_t;
typedef smol_vector(de_vertex_t) de_vertex_list_t;
typedef smol_vector(size_t) de_index_list_t;

void _parse_v3(const char* txt, de_v3_list_t* out) {
	char* ptr = txt;
	while (*ptr) {
		smol_v3_t vec = { 0 };
		vec.x = strtof(ptr, &ptr);
		vec.y = strtof(ptr, &ptr);
		vec.z = strtof(ptr, &ptr);
		smol_vector_push(out, vec);

		while (isspace(*ptr)) ptr++;
	}
}

void _parse_v2(const char* txt, de_v2_list_t* out) {
	char* ptr = txt;
	while (*ptr) {
		smol_v2_t vec = { 0 };
		vec.x = strtof(ptr, &ptr);
		vec.y = 1.0f - strtof(ptr, &ptr);
		smol_vector_push(out, vec);

		while (isspace(*ptr)) ptr++;
	}
}

// this will only load ONE mesh...
de_mesh_t de_mesh_load(smol_xml_t* xml, smol_xml_node_t* mesh_node) {
	// read triangles first
	// with this we can refer to the other data
#define check(c) (isalnum(c) || c == '_' || c == '-')
#define ltrim_if_not(x) do { while (*x && !check(*x)) x++; } while(0)

	smol_xml_matcher_t mt = { 0 };

	mt.has_tag = "triangles";
	smol_xml_node_t* triangles = smol_xml_find_one_child(xml, mesh_node, &mt);

	mt.has_tag = "input";
	mt.has_attr = "semantic";
	mt.attr_equals = "VERTEX";
	smol_xml_node_t* vertex = smol_xml_find_one_child(xml, triangles, &mt);

	mt.attr_equals = "NORMAL";
	smol_xml_node_t* normal = smol_xml_find_one_child(xml, triangles, &mt);

	mt.attr_equals = "TEXCOORD";
	smol_xml_node_t* tex_coord = smol_xml_find_one_child(xml, triangles, &mt);

	SMOL_ASSERT(vertex != NULL);
	SMOL_ASSERT(normal != NULL);
	SMOL_ASSERT(tex_coord != NULL);

	const char* vertex_data_src = smol_xml_get_attr(vertex, "source");
	int vertex_offset = atoi(smol_xml_get_attr(vertex, "offset"));
	const char* normal_data_src = smol_xml_get_attr(normal, "source");
	int normal_offset = atoi(smol_xml_get_attr(normal, "offset"));
	const char* tex_coord_data_src = smol_xml_get_attr(tex_coord, "source");
	int tex_coord_offset = atoi(smol_xml_get_attr(tex_coord, "offset"));

	// skip # if any
	ltrim_if_not(vertex_data_src);
	ltrim_if_not(normal_data_src);
	ltrim_if_not(tex_coord_data_src);

	de_v3_list_t positions;
	de_v3_list_t normals;
	de_v2_list_t tex_coords;
	smol_vector_init(&positions, 2000);
	smol_vector_init(&normals, 2000);
	smol_vector_init(&tex_coords, 2000);

	// parse vertex positions
	{
		mt.has_tag = "vertices";
		mt.has_attr = "id";
		mt.attr_equals = vertex_data_src;
		smol_xml_node_t* vertices = smol_xml_find_one_child(xml, mesh_node, &mt);
		SMOL_ASSERT(vertices != NULL);

		mt.has_tag = "input";
		mt.has_attr = "semantic";
		mt.attr_equals = "POSITION";
		smol_xml_node_t* position = smol_xml_find_one_child(xml, vertices, &mt);
		SMOL_ASSERT(position != NULL);

		vertex_data_src = smol_xml_get_attr(position, "source");
		ltrim_if_not(vertex_data_src);

		mt.has_tag = "source";
		mt.has_attr = "id";
		mt.attr_equals = vertex_data_src;
		smol_xml_node_t* source = smol_xml_find_one_child(xml, mesh_node, &mt);

		mt.has_tag = "float_array";
		mt.has_attr = NULL;
		mt.attr_equals = NULL;
		smol_xml_node_t* float_array = smol_xml_find_one_child(xml, source, &mt);

		_parse_v3(smol_xml_node_get_body(float_array), &positions);
	}

	// parse vertex normals
	{
		mt.has_tag = "source";
		mt.has_attr = "id";
		mt.attr_equals = normal_data_src;
		smol_xml_node_t* normals_node = smol_xml_find_one_child(xml, mesh_node, &mt);

		mt.has_tag = "float_array";
		mt.has_attr = NULL;
		mt.attr_equals = NULL;
		smol_xml_node_t* float_array = smol_xml_find_one_child(xml, normals_node, &mt);

		_parse_v3(smol_xml_node_get_body(float_array), &normals);
	}

	// parse tex coords
	{
		mt.has_tag = "source";
		mt.has_attr = "id";
		mt.attr_equals = tex_coord_data_src;
		smol_xml_node_t* tex_coord_node = smol_xml_find_one_child(xml, mesh_node, &mt);

		mt.has_tag = "float_array";
		mt.has_attr = NULL;
		mt.attr_equals = NULL;
		smol_xml_node_t* float_array = smol_xml_find_one_child(xml, tex_coord_node, &mt);

		_parse_v2(smol_xml_node_get_body(float_array), &tex_coords);
	}

	smol_vector(de_vertex_t) verts;
	smol_vector(unsigned int) inds;

	smol_vector_init(&verts, 2000);
	smol_vector_init(&inds, 2000);

	// parse indices
	{
		mt.has_tag = "p";
		mt.has_attr = NULL;
		mt.attr_equals = NULL;
		smol_xml_node_t* p = smol_xml_find_one_child(xml, triangles, &mt);

		int i = 0;
		char* txt = smol_xml_node_get_body(p);
		while (*txt) {
			de_vertex_t v = { 0 };
			for (size_t j = 0; j < 3; j++) {
				size_t index = strtoul(txt, &txt, 10);
				if (j == vertex_offset) {
					v.position = smol_vector_at(&positions, index);
				}
				else if (j == normal_offset) {
					v.normal = smol_vector_at(&normals, index);
				}
				else if (j == tex_coord_offset) {
					v.tex_coord = smol_vector_at(&tex_coords, index);
				}

				while (isspace(*txt)) txt++;
			}

			smol_vector_push(&verts, v);
			smol_vector_push(&inds, i);

			i++;
		}
	}

	// TODO: Bones, animations, etc...

	de_mesh_t mesh = { 0 };
	mesh.element_count = smol_vector_count(&inds);

	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(de_vertex_t) * smol_vector_count(&verts),
		smol_vector_data(&verts),
		GL_STATIC_DRAW
	);

#define off(s,m) ((size_t)&(((s*)0)->m))

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(de_vertex_t), off(de_vertex_t, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(de_vertex_t), off(de_vertex_t, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(de_vertex_t), off(de_vertex_t, tex_coord));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(de_vertex_t), off(de_vertex_t, bone_weights));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4,   GL_INT, GL_FALSE, sizeof(de_vertex_t), off(de_vertex_t, bone_ids));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int) * smol_vector_count(&inds),
		smol_vector_data(&inds),
		GL_STATIC_DRAW
	);

	glBindVertexArray(0);

	smol_vector_free(&verts);
	smol_vector_free(&inds);
	smol_vector_free(&positions);
	smol_vector_free(&normals);
	smol_vector_free(&tex_coords);

	return mesh;
#undef off
}

GLuint _create_shader_part(const char* src, GLenum type) {
	const char** src_ptr = &src;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, src_ptr, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char info_log[1024];
		
		glGetShaderInfoLog(shader, 1024, NULL, info_log);
		printf("%s\n", info_log);

		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

de_shader_t de_shader_create(const char* vs, const char* fs) {
	GLuint vs_part = _create_shader_part(vs, GL_VERTEX_SHADER);
	GLuint fs_part = _create_shader_part(fs, GL_FRAGMENT_SHADER);

	SMOL_ASSERT(vs_part != 0 && fs_part != 0);

	de_shader_t shader = { 0 };
	shader.program = glCreateProgram();
	glAttachShader(shader.program, vs_part);
	glAttachShader(shader.program, fs_part);
	glLinkProgram(shader.program);

	GLint status;
	glGetProgramiv(shader.program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		glDeleteProgram(shader.program);
		shader.program = 0;
	}

	glDetachShader(shader.program, vs_part);
	glDetachShader(shader.program, fs_part);
	glDeleteShader(vs_part);
	glDeleteShader(fs_part);

	return shader;
}

void de_shader_bind(de_shader_t* shader) {
	glUseProgram(shader ? shader->program : 0);
}

void de_shader_uniform1i(de_shader_t* shader, const char* name, int v) {
	SMOL_ASSERT(shader != NULL);
	GLint loc = glGetUniformLocation(shader->program, name);
	if (loc >= 0) glUniform1i(loc, v);
}

void de_shader_uniform1f(de_shader_t* shader, const char* name, float v) {
	SMOL_ASSERT(shader != NULL);
	GLint loc = glGetUniformLocation(shader->program, name);
	if (loc >= 0) glUniform1f(loc, v);
}

void de_shader_uniform2f(de_shader_t* shader, const char* name, smol_v2_t v) {
	SMOL_ASSERT(shader != NULL);
	GLint loc = glGetUniformLocation(shader->program, name);
	if (loc >= 0) glUniform2f(loc, v.x, v.y);
}

void de_shader_uniform3f(de_shader_t* shader, const char* name, smol_v3_t v) {
	SMOL_ASSERT(shader != NULL);
	GLint loc = glGetUniformLocation(shader->program, name);
	if (loc >= 0) glUniform3f(loc, v.x, v.y, v.z);
}

void de_shader_uniform4f(de_shader_t* shader, const char* name, smol_v4_t v) {
	SMOL_ASSERT(shader != NULL);
	GLint loc = glGetUniformLocation(shader->program, name);
	if (loc >= 0) glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void de_shader_uniform_m4f(de_shader_t* shader, const char* name, smol_m4_t v) {
	SMOL_ASSERT(shader != NULL);
	GLint loc = glGetUniformLocation(shader->program, name);
	if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_TRUE, v.m);
}

de_texture_t de_texture_create(const void* rgba_data, GLuint width, GLuint height) {
	de_texture_t tex;
	tex.width = width;
	tex.height = height;
	glGenTextures(1, &tex.id);

	glBindTexture(GL_TEXTURE_2D, tex.id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

de_renderer_t de_renderer_create() {
	de_renderer_t ren;
	smol_vector_init(&ren.drawables, 8192);
	ren.num_bindings = 0;
	ren.num_draw_calls = 0;
	return ren;
}

void de_renderer_destroy(de_renderer_t* renderer) {
	smol_vector_free(&renderer->drawables);
}

void de_renderer_submit(de_renderer_t* renderer, de_drawable_t drawable) {
	smol_vector_push(&renderer->drawables, drawable);
}

int _compare_drawables(const void* a, const void* b, void* ud) {
	return ((de_drawable_t*)a)->identifier < ((de_drawable_t*)b)->identifier;
}

void de_renderer_flush(de_renderer_t* renderer) {
	renderer->num_bindings = 0;
	renderer->num_draw_calls = 0;

	// sort drawables
	smol_sort_array(&renderer->drawables, &_compare_drawables);

	smol_vector(GLenum) saved_state;
	smol_vector_init(&saved_state, 128);

	size_t previous_drawable = -1;
	smol_vector_each(&renderer->drawables, de_drawable_t, dw) {
		if (previous_drawable != dw->identifier) {
			if (dw->blend_dfactor != 0 && dw->blend_sfactor != 0) {
				if (!glIsEnabled(GL_BLEND)) {
					smol_vector_push(&saved_state, GL_BLEND);
					glEnable(GL_BLEND);
				}
				glBlendFunc(dw->blend_sfactor, dw->blend_dfactor);
			}

			if (dw->depth_testing && !glIsEnabled(GL_DEPTH_TEST)) {
				smol_vector_push(&saved_state, GL_DEPTH_TEST);
				glEnable(GL_DEPTH_TEST);
			}

			if (dw->cull_face && !glIsEnabled(GL_CULL_FACE)) {
				smol_vector_push(&saved_state, GL_CULL_FACE);
				glEnable(GL_CULL_FACE);
				glCullFace(dw->cull_face);
			}

			de_shader_bind(dw->shader);
			renderer->num_bindings++;

			size_t slot = 0;
			for (size_t i = 0; i < DE_DRAWABLE_TEXTURES_COUNT; i++) {
				char uni_name[64] = { 0 };
				sprintf(uni_name, "unf_tex_%lu_active", slot);
				de_shader_uniform1i(dw->shader, uni_name, (dw->textures[i] != NULL));

				if (dw->textures[i]) {
					glActiveTexture(GL_TEXTURE0 + slot);
					glBindTexture(GL_TEXTURE_2D, dw->textures[i]->id);
					renderer->num_bindings++;

					sprintf(uni_name, "unf_tex_%lu", slot);
					de_shader_uniform1i(dw->shader, uni_name, slot);

					slot++;
				}
			}
		}

		if (dw->shader_setup) {
			dw->shader_setup(dw->shader, dw->user_data);
		}

		de_shader_uniform_m4f(dw->shader, "unf_model", dw->model);
		de_shader_uniform_m4f(dw->shader, "unf_view", renderer->view);
		de_shader_uniform_m4f(dw->shader, "unf_projection", renderer->projection);

		if (dw->mesh) {
			glBindVertexArray(dw->mesh->vao);
			renderer->num_bindings++;

			glDrawElements(GL_TRIANGLES, dw->mesh->element_count, GL_UNSIGNED_INT, NULL);
			renderer->num_draw_calls++;
		}

		smol_vector_each(&saved_state, GLenum, e) {
			glDisable(*e);
		}

		previous_drawable = dw->identifier;
	}

	smol_vector_free(&saved_state);
	smol_vector_clear(&renderer->drawables);
}
