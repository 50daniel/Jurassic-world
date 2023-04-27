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

// uniform float time;
// uniform int numWaves;
// uniform float amplitude;
// uniform float wavelength;
// uniform float speed;
uniform float time;
uniform sampler2D waterHeight;
uniform sampler2D waterheight1;
uniform sampler2D waveNormal;
uniform int mode;
void main()
{


    // vec4 info = texture2D(waterHeight, totaltextureCoodinate);
    //  vec2 dx = vec2(0.002, 0.0);
    // vec2 dy = vec2(0.0, 0.002);
    // float average = (
    // texture2D(waterHeight, totaltextureCoodinate - dx).r + //texture2D(u_material.texture_displacement, coord - dx).r * 0.1 +
    // texture2D(waterHeight, totaltextureCoodinate - dy).r + //texture2D(u_material.texture_displacement, coord - dy).r * 0.1+
    // texture2D(waterHeight, totaltextureCoodinate + dx).r + //texture2D(u_material.texture_displacement, coord + dx).r * 0.1+ 
    // texture2D(waterHeight, totaltextureCoodinate + dy).r //+ texture2D(u_material.texture_displacement, coord + dy).r * 0.1
    // ) * 0.25;
  

    // /* change the velocity to move toward the average */
    // info.g += (average - info.r) * 2.0;
    // // info.g += (average - (info.r + height.r * 0.1)) * 2.0;
    
    // /* attenuate the velocity a little so waves do not last forever */
    // info.g *= 0.995;
    
    // /* move the vertex along the velocity */
    // info.r += info.g;
    // // info.r = height.r * 0.1;


   // fragColor = info;
float hight;

if(mode ==0)
{
   
     gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);
     v_out.position = vec3(u_model * vec4(position, 1.0f));
    v_out.normal = mat3(transpose(inverse(u_model))) * normal;
    v_out.texture_coordinate =vec2(texture_coordinate.x, 1.0- texture_coordinate.y);
}
if(mode == 1)
{
        vec2 tmpcood=vec2(texture_coordinate.x,  1.0-texture_coordinate.y);
        vec2 textureCoodinate1 = vec2(texture_coordinate.x+time,  1.0-texture_coordinate.y);
    vec2 textureCoodinate2 = vec2(texture_coordinate.x+time, 1.0- texture_coordinate.y+time);
    vec2 totaltextureCoodinate= textureCoodinate1+textureCoodinate2;

    vec3 tmpPos=position;
    vec3 tmpNor=normal;
    vec3 color = vec3(texture(waterHeight, totaltextureCoodinate));
    tmpPos.y=color.r*10;
     //tmpPos.y = texture(waterHeight, textureCoodinate).r;
   
    tmpNor=vec3(texture(waveNormal, totaltextureCoodinate));
    vec3 o_normal=vec3(tmpNor.x,tmpNor.z,tmpNor.y);
    o_normal=normalize(o_normal);
    gl_Position = u_projection * u_view * u_model * vec4(tmpPos, 1.0f);
    v_out.position = vec3(u_model * vec4(tmpPos, 1.0f));
    v_out.normal = mat3(transpose(inverse(u_model))) * o_normal;
    v_out.texture_coordinate =vec2(totaltextureCoodinate.x, 1.0- totaltextureCoodinate.y);
    
}
else 
{
  vec3 tmpPos=position;
    vec3 tmpNor=normal;


    // vec2 tmpcood=vec2(texture_coordinate.x,  1.0-texture_coordinate.y);
    // vec2 textureCoodinate1 = vec2(texture_coordinate.x+time,  1.0-texture_coordinate.y+2*time);
    //  vec3 color1 = vec3(texture(waterheight1, textureCoodinate1));
    // tmpPos.y+=color1.r*10;
    //vec2 textureCoodinate2 = vec2(texture_coordinate.x+time, 1.0- texture_coordinate.y+time);
    //vec2 totaltextureCoodinate= textureCoodinate1+textureCoodinate2;

    // vec3 tmpPos=position;
    // vec3 tmpNor=normal;
    // vec3 color = vec3(texture(waterHeight, totaltextureCoodinate));
    // tmpPos.y=color.r*10;
    //  //tmpPos.y = texture(waterHeight, textureCoodinate).r;

    



  
    vec3 color = vec3(texture(waterHeight, texture_coordinate));
    tmpPos.y=color.r*30;
     //tmpPos.y = texture(waterHeight, textureCoodinate).r;
   
    tmpNor=vec3(texture(waveNormal, texture_coordinate));
    vec3 o_normal=vec3(tmpNor.x,tmpNor.z,tmpNor.y);
    o_normal=normalize(o_normal);
    gl_Position = u_projection * u_view * u_model * vec4(tmpPos, 1.0f);
    v_out.position = vec3(u_model * vec4(tmpPos, 1.0f));
    v_out.normal = mat3(transpose(inverse(u_model))) * o_normal;
    v_out.texture_coordinate =vec2(texture_coordinate.x,  texture_coordinate.y);
    
}

//totaltextureCoodinate=tmpcood;
  
   // clipSpace= u_projection * u_view * u_model * vec4(tmpPos, 1.0f);


}