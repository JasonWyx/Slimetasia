#version 410 core

// G-Buffers
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;
uniform sampler2D gSSAO;

// Lighting variables
uniform vec3 gCameraPosition;
uniform vec2 gFogAtten; // Near, Far
uniform vec3 gFogColor;
uniform vec3 gLightColor;

layout(location = 0) in vec2 iTexCoords;

out vec4 outColor;

float FogAttenuation(float dist)
{
  return clamp( (gFogAtten.y - dist) / (gFogAtten.y - gFogAtten.x), 0.0, 1.0);
}

void main()
{
  vec3 position   = texture(gPosition, iTexCoords).xyz;
  vec3 normal     = texture(gNormal, iTexCoords).xyz;
  vec4 diffuse    = texture(gDiffuse, iTexCoords);
  float ambientOcclusion = texture(gSSAO, iTexCoords).r;

  vec3 V = gCameraPosition - position;
  vec3 Vnorm = normalize(V);
  float Vdist = length(V);
  float fogAtten = FogAttenuation(Vdist);

  vec3 diffuseColor = (diffuse.rgb * gLightColor * ambientOcclusion);
       diffuseColor = fogAtten * diffuseColor + (1 - fogAtten) * gFogColor;
  outColor = vec4(diffuseColor, diffuse.a);
}