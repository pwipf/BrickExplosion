#version 330


in vec3 position;
in vec3 color;
in vec3 normal;
in mat4 instanceMat;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


out vec3 fcolor;
out vec3 fnorm;
out vec3 fpos;

void main() {
    gl_Position = projection * view * instanceMat *model *  vec4(position, 1);
    fcolor = color;

    fpos=vec3(view*instanceMat*model*vec4(position,1));
    fnorm=vec3(transpose(inverse(view*instanceMat*model)) *vec4(normal,0)       );
}
