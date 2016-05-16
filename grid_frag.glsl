#version 330

out vec4 color_out;
in vec3 fcolor;

void main() {
  color_out = vec4(fcolor,1);
}