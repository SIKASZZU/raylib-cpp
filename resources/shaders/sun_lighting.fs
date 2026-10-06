#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragWorldPosition;
in vec3 fragWorldNormal;
in vec4 fragLightPosition;

uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform vec4 colDiffuse;
uniform vec3 lightDirection;
uniform vec2 shadowTexelSize;

out vec4 finalColor;

float UnpackDepth(vec4 packedDepth)
{
    return dot(packedDepth, vec4(
        1.0 / (256.0 * 256.0 * 256.0),
        1.0 / (256.0 * 256.0),
        1.0 / 256.0,
        1.0));
}

float ShadowVisibility(vec3 normal, vec3 lightDir)
{
    vec3 projected = fragLightPosition.xyz / fragLightPosition.w;
    vec3 shadowCoord = projected * 0.5 + 0.5;

    if (shadowCoord.x < 0.0 || shadowCoord.x > 1.0 ||
        shadowCoord.y < 0.0 || shadowCoord.y > 1.0 ||
        shadowCoord.z < 0.0 || shadowCoord.z > 1.0)
        return 1.0;

    float bias = max(0.0012 * (1.0 - dot(normal, lightDir)), 0.0004);
    float visibility = 0.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 sampleUv = shadowCoord.xy + vec2(x, y) * shadowTexelSize;
            float closestDepth = UnpackDepth(texture(shadowMap, sampleUv));
            visibility += (shadowCoord.z - bias <= closestDepth) ? 1.0 : 0.0;
        }
    }
    return visibility / 9.0;
}

void main()
{
    vec4 albedo = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    vec3 normal = normalize(fragWorldNormal);
    vec3 lightDir = normalize(lightDirection);
    float diffuse = max(dot(normal, lightDir), 0.0);
    float visibility = ShadowVisibility(normal, lightDir);
    vec3 lighting = vec3(0.24) + vec3(1.0, 0.91, 0.72) * diffuse * visibility;

    // The scene's golden sphere represents the light source and should glow, not shade itself.
    if (albedo.r > 0.9 && albedo.g > 0.65 && albedo.b < 0.2)
        lighting = vec3(1.0);

    finalColor = vec4(albedo.rgb * lighting, albedo.a);
}
