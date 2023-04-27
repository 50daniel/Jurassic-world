#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;

uniform mat4 u_model;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   
} v_out;
out vec4 clipSpace;

const float pi = 3.14159;
uniform float waterHeight;
uniform float time;
uniform int numWaves;
uniform float amplitude[8];
uniform float wavelength[8];
uniform float speed[8];
uniform vec2 direction[8];


float wave(int i, float x, float y);

float waveHeight(float x, float y);

float dWavedx(int i, float x, float y);
float dWavedy(int i, float x, float y);
vec3 waveNormal(float x, float y);

void main()
{
   
    vec3 tmpPos=position;
    vec3 tmpNor=normal;
    tmpPos.y=waveHeight(position.x,position.z);
    tmpNor=waveNormal(position.x,position.z);
    gl_Position = u_projection * u_view * u_model * vec4(tmpPos, 1.0f);
    v_out.position = vec3(u_model * vec4(tmpPos, 1.0f));
    v_out.normal = mat3(transpose(inverse(u_model))) * tmpNor;
    v_out.texture_coordinate = vec2(texture_coordinate.x, 1.0f - texture_coordinate.y);
    
    clipSpace= u_projection * u_view * u_model * vec4(tmpPos, 1.0f);
}

float wave(int i, float x, float y) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    return amplitude[i] * sin(theta * frequency + time * phase);
}

float waveHeight(float x, float y) {
    float height = 0.0;
    for (int i = 0; i < numWaves; ++i)
        height += wave(i, x, y);
    return height;
}

float dWavedx(int i, float x, float y) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    float A = amplitude[i] * direction[i].x * frequency;
    return A * cos(theta * frequency + time * phase);
}

float dWavedy(int i, float x, float y) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    float A = amplitude[i] * direction[i].y * frequency;
    return A * cos(theta * frequency + time * phase);
}

vec3 waveNormal(float x, float y) {
    float dx = 0.0;
    float dy = 0.0;
    for (int i = 0; i < numWaves; ++i) {
        dx += dWavedx(i, x, y);
        dy += dWavedy(i, x, y);
    }
    vec3 n = vec3(-dx, 1.0, -dy);
    return normalize(n);
}