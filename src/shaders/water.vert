#version 430 core

layout (location = 0) in vec3 position;
out vec4 clipSpace;

uniform mat4 u_model;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

void main()
{
    
     clipSpace= u_projection * u_view * u_model * vec4(position, 1.0f);
      gl_Position = clipSpace;
}
