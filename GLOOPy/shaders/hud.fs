#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture;

void main()
{    
    vec4 texColor = texture(texture, TexCoords);
    if(texColor.r > 0.95 && texColor.g > 0.95 && texColor.b > 0.95)
        discard;
    FragColor = texColor;
}
