#version 430 core
layout (location = 0) in vec3 position;


uniform mat4 u_model;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

out vec3 skyboxDir;
uniform int mode;

void main()
{   
    vec3 tmpPos=position;
    
    if(mode==1)
    {
        tmpPos.y=-position.y;
    }
  

     skyboxDir = position;
     mat4 new_viewMat = u_view; 
     new_viewMat[3][0] = new_viewMat[3][1]= new_viewMat[3][2]=0;
    vec4 outPos = u_projection * new_viewMat  * vec4(tmpPos, 1.0f);
    gl_Position = outPos.xyww;
}