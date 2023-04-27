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

 //  vec3 color;
} v_out;

uniform vec4 plane1;
uniform vec4 plane2;
uniform int mode;
uniform vec3 viewPos;

void main()
{
  

    vec4 worldPos=u_model * vec4(position, 1.0);

    vec4 tmpPos=worldPos;
   if(mode==1)
   {
       tmpPos.y=-worldPos.y;
       if(worldPos.y*viewPos.y>0)
       {
           worldPos.y=-worldPos.y;
       }
        gl_ClipDistance[0]=dot (worldPos,plane1);
   }
    else if(mode == 2)
    {
        worldPos.y=-worldPos.y;
        if(worldPos.y*viewPos.y<0)
       {
           worldPos.y=100;
       }
         gl_ClipDistance[1]=dot (worldPos,plane2);
    }
   
  


    v_out.normal = mat3(transpose(inverse(u_model))) * normal;
    v_out.position = vec3(tmpPos);
    v_out.texture_coordinate=texture_coordinate;
   gl_Position = u_projection * u_view * tmpPos;
}  