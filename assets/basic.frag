#version 330 core

out vec4 fragColor;

in vec3 o_nrm;
in vec2 o_uv;

uniform sampler2D unf_tex_0;

uniform int unf_tex_0_active;
uniform int unf_tex_1_active;
uniform int unf_tex_2_active;

void main() {
	vec4 color = unf_tex_0_active > 0 ? texture2D(unf_tex_0, o_uv) : vec4(1.0);
	fragColor = color;
}
