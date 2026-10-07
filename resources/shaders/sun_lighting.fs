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
uniform vec3 viewPos;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

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
    float cosTheta = max(dot(normal, toLight), 0.0);
    float bias = max(0.0002 * (1.0 - cosTheta), 0.00002) + 0.00001;

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
    vec3 toLight = normalize(lightDirection);

    if (showShadowMask == 2)
    {
        // Debug: raw shadow map at this pixel's position in the sun's view (red = outside the box).
        vec3 sc = (fragLightPosition.xyz / fragLightPosition.w) * 0.5 + 0.5;
        bool outside = any(lessThan(sc, vec3(0.0))) || any(greaterThan(sc, vec3(1.0)));
        float stored = texture(shadowMap, sc.xy).r;
        finalColor = outside ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(vec3(stored), 1.0);
        return;
    }

    float visibility = ShadowVisibility(normal, toLight);

    if (showShadowMask == 1)
    {
        finalColor = vec4(vec3(step(0.5, visibility)), 1.0);
        return;
    }

    float diffuse = max(dot(normal, toLight), 0.0);
    vec3 ambient = vec3(0.30);
    vec3 direct = vec3(0.95, 0.87, 0.70) * diffuse * visibility;
    vec3 lit = albedo.rgb * (ambient + direct);

    float dist = length(viewPos - fragWorldPosition);
    float fog = smoothstep(fogStart, fogEnd, dist);

    finalColor = vec4(mix(lit, fogColor, fog), albedo.a);
}
