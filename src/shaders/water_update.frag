#version 430 core
out vec4 fragColor;
  
in vec2 TexCoords;

//layout (location = 0) out vec4 fragColor;

//#if defined(WATER_INTERACTIVE)
uniform sampler2D u_water;
uniform vec2 u_delta;

const float PI = 3.141592653589793;

uniform vec2 u_center;
uniform float u_radius;
uniform float u_strength;
uniform int drop;

void main() {

   vec4 info = texture2D(u_water, TexCoords);

    if(drop==1)
    {
        // if(length(u_center - TexCoords)<0.001)
        // {
        //     info.r=1.0;
        // }
        // else 
        // {
        //     info.r =0.0;
        // }
         /* add the drop to the height */
        float drop = max(0.0, 1.0 - length(u_center - TexCoords) / u_radius);
        drop = 0.5 - cos(drop * PI) * 0.5;
        info.r += drop * u_strength;
         fragColor = info;
      //  info=vec4(0.0,0.0,0.0,1.0);
    }
   

 else {
       /* get vertex info */
  //  vec4 info = texture2D(u_water, TexCoords);
    
    /* calculate average neighbor height */
    vec2 dx = vec2(u_delta.x, 0.0);
    vec2 dy = vec2(0.0, u_delta.y);
    float average = (
    texture2D(u_water, TexCoords - dx).r + //texture2D(u_material.texture_displacement, coord - dx).r * 0.1 +
    texture2D(u_water, TexCoords - dy).r + //texture2D(u_material.texture_displacement, coord - dy).r * 0.1+
    texture2D(u_water, TexCoords + dx).r + //texture2D(u_material.texture_displacement, coord + dx).r * 0.1+ 
    texture2D(u_water, TexCoords + dy).r //+ texture2D(u_material.texture_displacement, coord + dy).r * 0.1
    ) * 0.25;
  

    /* change the velocity to move toward the average */
    info.g += (average - info.r) * 1.0;
    // info.g += (average - (info.r + height.r * 0.1)) * 2.0;
    
    /* attenuate the velocity a little so waves do not last forever */
    info.g *= 0.95;
    
    /* move the vertex along the velocity */
    info.r += info.g;
    info.r *= 0.9;
    // info.r = height.r * 0.1;

    fragColor = info;
 }
 
}