#version 430 core
out vec4 f_color;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;

 //  vec3 color;
} f_in;


struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
uniform Material material;

// DirLight
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);



uniform vec3 u_color;


uniform vec3 viewPos;
uniform samplerCube skybox;
uniform sampler2D waterHeight;
uniform sampler2D waveNormal;
uniform sampler2D uvmap;
uniform vec2 u_delta;
uniform int mode;
vec2 textureOffset(vec2 uv, vec2 offset) {
    return texture(waterHeight, uv + offset);
}

void main()
{
    

    // //  //vec3 color = vec3(texture(u_texture, f_in.texture_coordinate));
  
    // /* calculate average neighbor height */
    // vec2 dx = vec2(u_delta.x, 0.0);
    // vec2 dy = vec2(0.0, u_delta.y);
    // float average = (
    // texture2D(waterHeight, f_in.texture_coordinate - dx).r + //texture2D(u_material.texture_displacement, coord - dx).r * 0.1 +
    // texture2D(waterHeight, f_in.texture_coordinate - dy).r + //texture2D(u_material.texture_displacement, coord - dy).r * 0.1+
    // texture2D(waterHeight, f_in.texture_coordinate + dx).r + //texture2D(u_material.texture_displacement, coord + dx).r * 0.1+ 
    // texture2D(waterHeight, f_in.texture_coordinate + dy).r //+ texture2D(u_material.texture_displacement, coord + dy).r * 0.1
    // ) * 0.25;
    
    // /* change the velocity to move toward the average */
    // info.g += (average - info.r) * 2.0;
    // // info.g += (average - (info.r + height.r * 0.1)) * 2.0;
    
    // /* attenuate the velocity a little so waves do not last forever */
    // info.g *= 0.995;
    
    // /* move the vertex along the velocity */
    // info.r += info.g;
    // // info.r = height.r * 0.1;

    // f_color = info;
if(mode ==0)
{
    vec3 color = vec3 (texture(uvmap, f_in.texture_coordinate));
       
      f_color =vec4(color,1.0);
    

}
else
{
     vec4 info = texture2D(waterHeight, f_in.texture_coordinate);
    float dx = 0.002;
    float dz = 0.002;

//     /* update the normal */
    float dy = texture2D(waterHeight, vec2(f_in.texture_coordinate.x + dx, f_in.texture_coordinate.y)).r - info.r;
    vec3 du = vec3(dx, dy * 0.1, 0.0);
    
    dy = texture2D(waterHeight, vec2(f_in.texture_coordinate.x, f_in.texture_coordinate.y + dz)).r - info.r;
    vec3 dv = vec3(0.0, dy * 0.1, dz);
    vec3 normal = normalize(cross(dv, du));
    f_color = vec4(normal, 1);

           vec3 myLight={0.0,0.0,0.0};
    //dirLight
      myLight += CalcDirLight(dirLight, normal, normalize(f_in.position - viewPos));
    f_color = vec4(myLight, 1.0f);



    vec3 I = normalize(f_in.position - viewPos);
    vec3 R = reflect(I, normalize(normal));
    f_color = vec4(texture(skybox, R).rgb, 1.0);
}


}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse,f_in.texture_coordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_in.texture_coordinate));
    return (ambient + diffuse + specular);
}  