#version 330 core
in vec3 WorldPosition;
in vec3 WorldNormal;

uniform vec3 objectColor;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
out vec4 FragColor;

void main() {
    vec3 normal = normalize(WorldNormal);
    float diffuse = max(dot(normal, normalize(lightDirection)), 0.0);
    vec3 litColor = objectColor * (vec3(0.25) + lightColor * diffuse * 0.72);

    float distanceToCamera = distance(cameraPosition, WorldPosition);
    float fog = clamp((distanceToCamera - 22.0) / 48.0, 0.0, 0.86);
    vec3 fogColor = vec3(0.018, 0.030, 0.068);
    litColor = mix(litColor, fogColor, fog);
    FragColor = vec4(litColor, 1.0);
}
