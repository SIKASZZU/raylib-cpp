#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in mat4 instanceTransform;

uniform mat4 mvp;
uniform mat4 lightViewProj;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragWorldPosition;
out vec3 fragWorldNormal;
out vec4 fragLightPosition;

void main()
{
    vec4 worldPosition = instanceTransform * vec4(vertexPosition, 1.0);

    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    fragWorldPosition = worldPosition.xyz;
    fragWorldNormal = normalize(mat3(instanceTransform) * vertexNormal);
    fragLightPosition = lightViewProj * worldPosition;

    gl_Position = mvp * worldPosition;
}
