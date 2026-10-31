#version 330 core
layout (location = 0) in vec3 aPos;

out vec2 TexCoords;

uniform mat4 projection;
uniform vec2 position;
uniform float size;

void main()
{
    TexCoords = aPos.xy;
    vec2 screenPos = position + (aPos.xy * size);
    gl_Position = projection * vec4(screenPos,0,1);
}
