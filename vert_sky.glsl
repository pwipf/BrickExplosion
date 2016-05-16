#version 330

uniform mat4 rotation;


in vec3 position;


out vec3 fuv;

void main() {
    gl_Position = rotation * vec4(position, 1.0);
    position.y;
    fuv=position;
}
