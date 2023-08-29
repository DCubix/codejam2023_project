#version 330 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_nrm;
layout (location = 2) in vec2 v_tex;

uniform mat4 unf_model;
uniform mat4 unf_view;
uniform mat4 unf_projection;

out vec3 o_nrm;
out vec2 o_uv;

void main() {
	gl_Position = unf_projection * unf_view * unf_model * vec4(v_pos, 1.0);
	o_nrm = v_nrm;
	o_uv = v_tex;
}
