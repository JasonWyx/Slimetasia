#version 410 core

const int KERNAL_SIZE = 64;
const float RADIUS    = 0.25;
const float BIAS      = 0.005;

layout(location = 0) in vec2 iTexCoords;

uniform sampler2D gPositionTexture;
uniform sampler2D gNormalTexture;
uniform sampler2D gTextureNoise;
uniform vec2 gNoiseScale;
uniform vec3 gSamples[64];

// Transform matrices
uniform mat4 gView;
uniform mat4 gProj;

out float oSSAOColor;

void main()
{
  vec3 position = (gView * vec4(texture(gPositionTexture, iTexCoords).xyz, 1.0)).xyz;
  vec3 normal   = normalize((gView * vec4(texture(gNormalTexture, iTexCoords).xyz, 0.0)).xyz);
  vec3 randomVector = normalize(texture(gTextureNoise, iTexCoords * gNoiseScale).xyz);

  vec3 tangent =  normalize(randomVector - normal * dot(randomVector, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, bitangent, normal);

  float occlusion = 0.0;
  for(int i = 0; i < KERNAL_SIZE; ++i)
  {
    vec3 kernal = TBN * gSamples[i];
    kernal = position + kernal * RADIUS;

    vec4 offset = vec4(kernal, 1.0);
    offset = gProj * offset;
    offset.xy /= offset.w;
    offset.xy = offset.xy * 0.5 + 0.5;

    float sampleDepth = (gView * vec4(texture(gPositionTexture, offset.xy).xyz, 1.0)).z;
    float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(position.z - sampleDepth));
    occlusion += (sampleDepth >= kernal.z + BIAS ? 1.0 : 0.0) * rangeCheck;
  }

  occlusion = 1.0 - (occlusion / KERNAL_SIZE);

  oSSAOColor = occlusion;
}