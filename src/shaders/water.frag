#version 430 core

in vec4 clipSpace;

out vec4 f_color;

uniform sampler2D reflectTexture;
uniform sampler2D refractTexture;

void main()
{
        vec2 ndc =(clipSpace.xy/clipSpace.w)/2.0 + 0.5;
    vec2 reflectTexCoords = vec2(ndc.x,ndc.y);
    vec2 refractTexCoords = vec2(ndc.x,ndc.y);

    vec4 reflectColor=texture(reflectTexture, reflectTexCoords);
    vec4 refractColor=texture(refractTexture, refractTexCoords);
  //  f_color = mix(reflectColor, refractColor, 0.5);
    f_color=vec4(vec3(reflectColor),1.0);
}