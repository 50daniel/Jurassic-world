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
  // vec3 color;
} v_out;

// uniform vec3 viewPos;

// struct Material
// {
//     sampler2D diffuse;
//     sampler2D specular;
//     float shininess;
// };
// uniform Material material;

// // DirLight
// struct DirLight {
//     vec3 direction;

//     vec3 ambient;
//     vec3 diffuse;
//     vec3 specular;
// };  
// uniform DirLight dirLight;
// vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

// //PointLight
// struct PointLight {    
//     vec3 position;

//     float constant;
//     float linear;
//     float quadratic;  

//     vec3 ambient;
//     vec3 diffuse;
//     vec3 specular;
// };  
// uniform PointLight pointLights;

// vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

// //spotLight
// struct SpotLight{
//     vec3 position;
//     vec3 direction;
//     float cutOff;
//     float outerCutOff;

//     float constant;
//     float linear;
//     float quadratic;  

//     vec3 ambient;
//     vec3 diffuse;
//     vec3 specular;
// };
// uniform SpotLight spotLight;
// vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);

    v_out.position = vec3(u_model * vec4(position, 1.0f));
    v_out.normal = mat3(transpose(inverse(u_model))) * normal;
    v_out.texture_coordinate = vec2(texture_coordinate.x, 1.0f - texture_coordinate.y);
   // v_out.color=CalcDirLight(dirLight, v_out.normal, normalize(v_out.position - viewPos));
    //v_out.color = CalcPointLight(pointLights, v_out.normal, v_out.position,  normalize(v_out.position - viewPos));
    // v_out.color = CalcSpotLight(spotLight, v_out.normal, v_out.position,  normalize(v_out.position - viewPos));
}

// vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
// {
//     vec3 lightDir = normalize(-light.direction);
//     // Diffuse shading
//     float diff = max(dot(normal, lightDir), 0.0);
//     // Specular shading
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//     // Combine results
//     vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, v_out.texture_coordinate));
//     vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse,v_out.texture_coordinate));
//     vec3 specular = light.specular * spec * vec3(texture(material.specular, v_out.texture_coordinate));
//     return (ambient + diffuse + specular);
// }  

// vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
// {
//     vec3 lightDir = normalize(light.position - fragPos);
//     // Diffuse shading
//     float diff = max(dot(normal, lightDir), 0.0);
//     // Specular shading
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//     // Attenuation
//     float distance    = length(light.position - fragPos);
//     float attenuation = 1.0f / (light.constant + light.linear * distance + 
//                  light.quadratic * (distance * distance));    
//     // Combine results
//     vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, v_out.texture_coordinate));
//     vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, v_out.texture_coordinate));
//     vec3 specular = light.specular * spec * vec3(texture(material.specular, v_out.texture_coordinate));
//     ambient  *= attenuation;
//     diffuse  *= attenuation;
//     specular *= attenuation;
//     return (ambient + diffuse + specular);
// } 

// vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
// {
//     vec3 lightDir = normalize(light.position - fragPos);
//     // Diffuse shading
//     float diff = max(dot(normal, lightDir), 0.0);
//     // Specular shading
//     vec3 reflectDir = reflect(-lightDir, normal);
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//     // Attenuation
//     float distance    = length(light.position - fragPos);
//     float attenuation = 1.0f / (light.constant + light.linear * distance + 
//                  light.quadratic * (distance * distance)); 

//     float theta     = dot(lightDir, normalize(-light.direction));
//     float epsilon   = light.cutOff - light.outerCutOff;
//     float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
//     // Combine results
//     vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, v_out.texture_coordinate));
//     vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, v_out.texture_coordinate));
//     vec3 specular = light.specular * spec * vec3(texture(material.specular, v_out.texture_coordinate));
//     ambient  *= intensity;
//     diffuse  *=intensity;
//     specular *= intensity;
//     return (ambient + diffuse + specular);
// } 