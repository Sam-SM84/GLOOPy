#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// HUD / screen-flash quads are drawn straight in clip space
uniform bool uOverlay;
uniform vec2 uOverlayScale;
uniform vec2 uOverlayOffset;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    FragPos   = world.xyz;
    Normal    = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    if (uOverlay)
        gl_Position = vec4(aPos.xy * uOverlayScale + uOverlayOffset, 0.0, 1.0);
    else
        gl_Position = projection * view * world;
}
