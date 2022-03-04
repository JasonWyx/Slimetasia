#version 410 core
#define MAX_BONES 32

layout(location = 0) in vec3  iPosition;
layout(location = 6) in ivec4 iJointIds;
layout(location = 7) in vec4  iJointWeights;

uniform mat4 gMVPTransform;
uniform mat4 gBoneTransforms[MAX_BONES];
uniform mat4 gNodeTransform;
uniform bool gIsAnimated;

void main()
{
  mat4 boneTransform;

  if(gIsAnimated)
  {
    boneTransform = gBoneTransforms[iJointIds[0]] * iJointWeights[0] +
                    gBoneTransforms[iJointIds[1]] * iJointWeights[1] +
                    gBoneTransforms[iJointIds[2]] * iJointWeights[2] +
                    gBoneTransforms[iJointIds[3]] * iJointWeights[3];
  }
  else
  {
    boneTransform = mat4(1.0);
  }

  gl_Position = gMVPTransform * boneTransform * gNodeTransform * vec4(iPosition, 1.0);
}