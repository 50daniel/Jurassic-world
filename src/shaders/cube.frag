#version 430 core
out vec4 f_color;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;

 //  vec3 color;
} f_in;

uniform vec3 u_color;

uniform sampler2D u_texture;
uniform vec3 viewPos;

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

//PointLight
struct PointLight {    
    vec3 position;

    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform PointLight pointLights;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

//spotLight
struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

uniform samplerCube skybox;

void main()
{   

    // vec3 myLight={0,0,0};
    // //dirLight
    // myLight += CalcDirLight(dirLight, f_in.normal, normalize(f_in.position - viewPos));


    vec3 color = vec3(texture(u_texture, f_in.texture_coordinate));
   
   
    //  color = CalcPointLight(pointLights, f_in.normal, f_in.position,  normalize(f_in.position - viewPos));
    //  //color = CalcSpotLight(spotLight, f_in.normal, f_in.position,  normalize(f_in.position - viewPos));
     f_color = vec4(color, 1.0f);

    //  vec3 I = normalize(f_in.position - viewPos);
    //  vec3 R = reflect(I, normalize(f_in.normal));
    //  f_color = vec4(texture(skybox, R).rgb, 1.0);

    // float ratio = 1.00 / 1.52;
    // vec3 I = normalize(f_in.position - viewPos);
    // vec3 R = refract(I, normalize(f_in.normal), ratio);
    // f_color = vec4(texture(skybox, R).rgb, 1.0);
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

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    // Combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_in.texture_coordinate));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // Attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance)); 

    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // Combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, f_in.texture_coordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, f_in.texture_coordinate));
    ambient  *= intensity;
    diffuse  *=intensity;
    specular *= intensity;
    return (ambient + diffuse + specular);
} 