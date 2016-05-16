#version 330 core

in vec3 fuv;
out vec4 color_out;
uniform float brightness;

uniform samplerCube samp;

void main() {
    vec4 color=texture(samp, fuv);
    color_out = vec4(color.r*brightness, color.g*brightness, color.b*brightness ,color.a);
    //color_out = texture(samp, fuv);
    //color_out = vec4(fuv,1.0);
}
