#version 410 core

// G-Buffers
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;

// Shadow variables
uniform sampler2D gShadowMap;
uniform mat4      gLightSpaceTransform;

// Lighting variables
uniform vec3 gCameraPosition;
uniform vec3 gLightAtten; // Quadratic, Linear, Constant
uniform vec2 gFogAtten; // Near, Far
uniform vec3 gFogColor;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float lightIntensity;

layout(location = 0) in vec2 iTexCoords;

out vec4 outColor;


float FogAttenuation(float dist)
{
  return clamp( (gFogAtten.y - dist) / (gFogAtten.y - gFogAtten.x), 0.0, 1.0);
}

float CalculateDiffuseContrib(vec3 worldNormal)
{
  vec3 L = -lightDirection;
  L = normalize(L);

  return max(dot(L, worldNormal), 0.0);
}

float CalculateShadowFactor(vec3 worldPosition, vec3 worldNormal)
{
  vec4 lightSpacePosition = gLightSpaceTransform * vec4(worldPosition, 1.0);
  lightSpacePosition /= lightSpacePosition.w;
  lightSpacePosition = lightSpacePosition * 0.5 + 0.5;

  if(lightSpacePosition.z > 1.0)
    return 1.0;

  float bias = max(0.005 * (1.0 - dot(worldNormal, -lightDirection)), 0.0005);
  float sampleDepth = texture(gShadowMap, lightSpacePosition.xy).r;

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(gShadowMap, 0);
  for(float x = -1; x <= 1; x += 0.5)
  {
    for(float y = -1; y <= 1; y += 0.5)
    {
      float pcfDepth = texture(gShadowMap, lightSpacePosition.xy + vec2(x, y) * texelSize).r; 
      shadow += lightSpacePosition.z - bias > pcfDepth ? 0.0 : 1.0;        
    }    
  }
  shadow /= 25.0;

  //if(sampleDepth < lightSpacePosition.z - bias)
  //  return 0.0;
  //return 1.0;
  return shadow;
}

void main()
{
  vec3 position   = texture(gPosition, iTexCoords).xyz;
  vec3 normal     = normalize(texture(gNormal, iTexCoords).xyz);
  vec4 diffuse    = texture(gDiffuse, iTexCoords);

  vec3 V          = gCameraPosition - position;
  vec3 Vnorm      = normalize(V);
  float Ldist     = length(V);
  float fogAtten  = FogAttenuation(Ldist);

  vec3 diffuseColor = (diffuse.rgb * lightColor * lightIntensity) * CalculateDiffuseContrib(normal);
  vec3 finalColor   = diffuseColor * CalculateShadowFactor(position, normal);
       //finalColor   = fogAtten * finalColor + (1 - fogAtten) * vec3(0);

  outColor = vec4(finalColor, diffuse.a);
}