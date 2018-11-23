#version 330

uniform sampler2D texture;

in vec2 v_coord;

out vec4 color;

void main() {
	color = texture2D(texture, v_coord);
}