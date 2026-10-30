#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;
uniform bool image;
uniform vec3 color;

void main()
{  
    if(image)
    {
        vec4 texColor = texture(texture1, TexCoords);
        if(texColor.r > 0.95 && texColor.g > 0.95 && texColor.b > 0.95)
            discard;
        FragColor = texColor;
    }

    else
    {
        FragColor = vec4(color,1.0);
    }  

}