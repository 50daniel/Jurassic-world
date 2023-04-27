#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 Normal;

uniform mat4 u_model;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

out vec3 Normal;
out vec3 Position;

void main()
{
    Normal = mat3(transpose(inverse(u_model))) * Normal;
    Position = vec3(u_model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(Position, 1.0);
}  