#version 460 core
layout (location = 0) in vec3 vIn_Position;
layout (location = 1) in float vIn_Distance;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform float u_AudioSamples[512];

out float vOut_F;

void main()
{
    int i = int(vIn_Distance * 512);
    vOut_F = u_AudioSamples[i];

    vec3 pos = vIn_Position.xyz;
    pos.y += u_AudioSamples[i] * 5;
    gl_Position = u_Proj * u_View * u_Model * vec4(pos.xyz, 1.0);
}
