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
uniform int showShadowMask;

out vec4 finalColor;

float ShadowVisibility(vec3 normal, vec3 toLight)
{
    vec3 projected = fragLightPosition.xyz / fragLightPosition.w;
    vec3 shadowCoord = projected * 0.5 + 0.5;

    // No Y flip here. The light-space projection and the sampled OpenGL depth
    // texture use the same texture-coordinate orientation.
    if (shadowCoord.x < 0.0 || shadowCoord.x > 1.0 ||
        shadowCoord.y < 0.0 || shadowCoord.y > 1.0 ||
        shadowCoord.z < 0.0 || shadowCoord.z > 1.0)
        return 1.0;

    // Slope-scaled bias. A 24-bit depth texture needs much less bias than the
    // old packed RGBA8 path, which helps prevent shadows from detaching.
    float bias = max(0.0002 * (1.0 - dot(normal, toLight)), 0.00002) + 0.00001;

    float visibility = 0.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 sampleUv = shadowCoord.xy + vec2(x, y) * shadowTexelSize;
            float closestDepth = texture(shadowMap, sampleUv).r;
            visibility += (shadowCoord.z - bias <= closestDepth) ? 1.0 : 0.0;
        }
    }

    return visibility / 9.0;
}

void main()
{
    vec4 albedo = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    vec3 normal = normalize(fragWorldNormal);

    // The ground mesh contains top-facing geometry; this keeps malformed/downward
    // floor normals from making the directional lighting misleading while debugging.
    if (normal.y < -0.95)
        normal = -normal;

    vec3 toLight = normalize(lightDirection);
    float visibility = ShadowVisibility(normal, toLight);

    if (showShadowMask != 0)
    {
        // White = visible to the sun, black = shadowed by the depth test.
        finalColor = vec4(vec3(step(0.5, visibility)), 1.0);
        return;
    }

    float diffuse = max(dot(normal, toLight), 0.0);
    vec3 ambient = vec3(0.30);
    vec3 direct = vec3(0.95, 0.87, 0.70) * diffuse * visibility;

    finalColor = vec4(albedo.rgb * (ambient + direct), albedo.a);
}
