#version 330 core

in vec2 TexCoords;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform sampler2D texture_diffuse1;
uniform bool light;

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if(light)
    {
        vec3 N = normalize(Normal);
        vec3 L = normalize(lightDirection);
        float diffuse = max(dot(N, L), 0.1);

        if(texColor.a < 0.5)
            discard;

        vec3 result = vec3(texColor) * diffuse * lightColor;

        FragColor = vec4(result,1.0f);
    }

    else
    {
        if(texColor.a < 0.5)
            discard;
        FragColor = texColor;
    }

}
