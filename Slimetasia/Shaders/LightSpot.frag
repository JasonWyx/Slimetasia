#version 410 core

// G-Buffers
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;

// Shadow variables
uniform sampler2D gShadowMap;
uniform mat4      gLightSpaceTransform;

uniform ivec2     gScreenSize;

// Lighting variables
uniform vec3 gCameraPosition;
uniform vec3 gLightAtten; // Quadratic, Linear, Constant
uniform vec2 gFogAtten; // Near, Far
uniform vec3 gFogColor;
uniform vec3 lightDirection;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float innerAngle;
uniform float outerAngle;
uniform float falloffAngle;

out vec4 outColor;

vec2 CalculateTexCoord()
{
  return gl_FragCoord.xy / gScreenSize;
}

float LightAttenuation(float dist)
{
  return max(0.0, lightIntensity / (gLightAtten.x * dist * dist + gLightAtten.y * dist + gLightAtten.z));
}

float FogAttenuation(float dist)
{
  return clamp( (gFogAtten.y - dist) / (gFogAtten.y - gFogAtten.x), 0.0, 1.0);
}

float CalculateDiffuseContrib(vec3 L, vec3 worldNormal)
{
  return max(dot(L, worldNormal), 0.0);
}

float CalculateSpecularContrib(vec3 L, vec3 N, vec3 V, float shininess)
{
  //vec3 H = normalize(L + V);
  //return pow(max(dot(H, N), 0.0f), shininess);

  vec3 R = normalize(reflect(-L, N));
  float RdotVclamped = max(dot(R, V), 0.0f);
  return (RdotVclamped == 0.0  || shininess == 0.0) ? 0.0 : pow(RdotVclamped, shininess);
}

float CalculateSpotlightEffect(vec3 L, vec3 D)
{
  float LdotDclamped = max(dot(-D, L), 0.0f);

  float SpotlightEffect = max(
          (LdotDclamped    - cos(outerAngle)) /
          (cos(innerAngle) - cos(outerAngle)), 0.0);

  return pow(SpotlightEffect, falloffAngle);
}

float CalculateShadowFactor(vec3 worldPosition, vec3 worldNormal)
{
  vec4 lightSpacePosition = gLightSpaceTransform * vec4(worldPosition, 1.0);
  lightSpacePosition /= lightSpacePosition.w;
  lightSpacePosition = lightSpacePosition * 0.5 + 0.5;

  if(lightSpacePosition.z > 1.0)
    return 0.0;

  float bias = max(0.005 * (1.0 - dot(worldNormal, -lightDirection)), 0.0005);
  float sampleDepth = texture(gShadowMap, lightSpacePosition.xy).r;

  if(sampleDepth < lightSpacePosition.z - bias)
    return 0.0;
  return 1.0;
}

void main()
{
  vec2 texCoord = CalculateTexCoord();
  
  vec3 position   = texture(gPosition, texCoord).xyz;
  vec3 normal     = texture(gNormal, texCoord).xyz;
  vec4 diffuse    = texture(gDiffuse, texCoord);
  vec3 specular   = texture(gSpecular, texCoord).xyz;
  float shininess = texture(gSpecular, texCoord).a;

  // Calculate light specific quantities
  vec3 L      = lightPosition - position;
  vec3 Lnorm  = normalize(L);
  float Ldist = length(L);

  // Calculate view specific quantities
  vec3 V      = gCameraPosition - position;
  vec3 Vnorm  = normalize(V);
  float Vdist = length(V);

  float fogAtten  = FogAttenuation(Vdist);
  vec3 Dnorm      = normalize(lightDirection);

  vec3 diffuseColor   = diffuse.rgb * lightColor * lightIntensity * CalculateDiffuseContrib(Lnorm, normal);
  vec3 specularColor  = specular * lightColor * lightIntensity * CalculateSpecularContrib(Lnorm, normal, Vnorm, shininess);
  vec3 finalColor     = (diffuseColor + specularColor) * CalculateShadowFactor(position, normal) * CalculateSpotlightEffect(Lnorm, Dnorm) * LightAttenuation(Ldist);
       //finalColor     = fogAtten * finalColor + (1 - fogAtten) * vec3(0);
  outColor = vec4(finalColor, diffuse.a);
}