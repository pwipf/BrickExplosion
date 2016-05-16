#version 330


in vec3 position;
in vec3 color;
in vec3 normal;
in vec2 uv;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 fcolor;
out vec3 fnorm;
out vec3 fpos;
out vec2 fuv;

void main() {
    gl_Position = projection * view *  model * vec4(position, 1);
    fcolor = color;

    fpos=vec3(view*model*vec4(position,1));
    fnorm=vec3(transpose(inverse(view*model)) * vec4(normal,0));
    fuv=uv;
}
