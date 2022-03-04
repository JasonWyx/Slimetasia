#version 410 core
#define MAX_BONES 32

layout(location = 0) in vec3  position;
layout(location = 5) in vec2  texCoords;
layout(location = 6) in ivec4 jointIds;
layout(location = 7) in vec4  jointWeights;

uniform bool gIsAnimated;
uniform mat4 gBoneTransforms[MAX_BONES];
uniform mat4 gNodeTransform;
uniform mat4 gMTransform;

layout(location = 0) out vec2 outTexCoords;

void main()
{
  if(gIsAnimated)
  {
    gl_Position = gMTransform * (gBoneTransforms[jointIds[0]] * jointWeights[0] +
                                 gBoneTransforms[jointIds[1]] * jointWeights[1] +
                                 gBoneTransforms[jointIds[2]] * jointWeights[2] +
                                 gBoneTransforms[jointIds[3]] * jointWeights[3] ) * vec4(position, 1.0);
  }
  else
  {
    gl_Position = gMTransform * gNodeTransform * vec4(position, 1.0);
  }
  
  outTexCoords = texCoords;
}