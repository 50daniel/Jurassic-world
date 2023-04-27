#version 430 core
out vec4 f_color;

in vec3 skyboxDir;

uniform vec3 u_color;

uniform samplerCube cubemap;
void main()
{   
    //vec3 color = vec3(texture(cubemap, skyboxDir));
    f_color =texture(cubemap, skyboxDir);
}