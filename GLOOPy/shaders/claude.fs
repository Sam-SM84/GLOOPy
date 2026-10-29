#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D texture_diffuse1;   // filled by OOGL's Mesh::Draw for textured models
uniform bool  uUseTexture;            // true = sample texture, false = flat uColor
uniform bool  uChecker;               // procedural checkerboard (ground)
uniform vec3  uColor;
uniform float uEmissive;              // 0 = fully lit, 1 = ignores lighting (glowing)

uniform vec3  lightDirection;         // points TOWARD the sun (same convention as OOGL's Light)
uniform vec3  lightColor;
uniform vec3  uViewPos;
uniform vec3  uFogColor;
uniform float uFogDensity;

uniform bool  uOverlay;               // HUD / flash quads
uniform vec3  uTint;
uniform float uTintAmount;

void main()
{
    if (uOverlay)
    {
        FragColor = vec4(uTint, uTintAmount);
        return;
    }

    vec4 base = uUseTexture ? texture(texture_diffuse1, TexCoords) : vec4(uColor, 1.0);
    if (base.a < 0.5)
        discard;

    if (uChecker)
    {
        vec2 c = floor(FragPos.xz / 2.0);
        float k = mod(c.x + c.y, 2.0);
        base.rgb *= mix(0.78, 1.0, k);
    }

    vec3 N = normalize(Normal);
    vec3 L = normalize(lightDirection);
    vec3 V = normalize(uViewPos - FragPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = (diff > 0.0) ? pow(max(dot(N, H), 0.0), 32.0) * 0.2 : 0.0;

    vec3 ambient = vec3(0.38, 0.42, 0.50);
    vec3 lit     = base.rgb * (ambient + diff * lightColor) + spec * lightColor;
    vec3 color   = mix(lit, base.rgb, uEmissive);

    float dist = length(uViewPos - FragPos);
    float fog  = 1.0 - exp(-pow(dist * uFogDensity, 2.0));
    color = mix(color, uFogColor, clamp(fog, 0.0, 1.0));

    FragColor = vec4(color, 1.0);
}
