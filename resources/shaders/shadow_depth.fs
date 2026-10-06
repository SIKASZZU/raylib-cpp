#version 330

out vec4 finalColor;

void main()
{
    float depth = gl_FragCoord.z;
    vec4 bitShift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0);
    vec4 bitMask = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
    vec4 packedDepth = fract(depth * bitShift);
    packedDepth -= packedDepth.xxyz * bitMask;
    finalColor = packedDepth;
}
