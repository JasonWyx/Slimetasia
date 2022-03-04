#version 410 core
#define MAX_BONES 32

layout(location = 0) in vec3  iPosition;
layout(location = 1) in vec3  iNormal;
layout(location = 2) in vec3  iTangent;
layout(location = 3) in vec3  iBitangent;
layout(location = 4) in vec3  iColor;
layout(location = 5) in vec2  iTexCoords;
layout(location = 6) in ivec4 iJointIds;
layout(location = 7) in vec4  iJointWeights;

uniform mat4 gModelTransform;
uniform mat4 gNodeTransform;
uniform mat4 gViewProjTransform;
uniform mat4 gBoneTransforms[MAX_BONES];
uniform bool gIsAnimated;
uniform vec4 gClipPlane;

// Tiling axis
const uint XY = 0;
const uint XZ = 1;
const uint YZ = 2;

uniform bool  gTilingEnabled;
uniform uint  gTilingAxis;
uniform float gTilingSize;

out VertData
{
  vec3 worldPosition;
  vec3 worldNormal;
  vec3 worldTangent;
  vec3 worldBitangent;
  vec3 vertColor;
  vec2 texCoord;

} vsout;

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

  vsout.worldPosition  = (gModelTransform * boneTransform * gNodeTransform * vec4(iPosition, 1.0)).xyz;
  vsout.worldNormal    = (gModelTransform * boneTransform * gNodeTransform * vec4(iNormal, 0.0)).xyz;
  vsout.worldTangent   = (gModelTransform * boneTransform * gNodeTransform * vec4(iTangent, 0.0)).xyz;
  vsout.worldBitangent = (gModelTransform * boneTransform * gNodeTransform * vec4(iBitangent, 0.0)).xyz;
  vsout.vertColor      = iColor;

  if(gTilingEnabled)
  {
    switch(gTilingAxis)
    {
      case XY:
      {
        vsout.texCoord = vec2(vsout.worldPosition.x, vsout.worldPosition.y) * gTilingSize;
      }
      break;

      case XZ:
      {
        vsout.texCoord = vec2(vsout.worldPosition.x, vsout.worldPosition.z) * gTilingSize;
      }
      break;

      case YZ:
      {
        vsout.texCoord = vec2(vsout.worldPosition.y, vsout.worldPosition.z) * gTilingSize;
      } 
      break;
    }
  }
  else
  {
    vsout.texCoord = iTexCoords;
  }

  // If clipping is enabled
  if(length(gClipPlane) != 0.0)
  {
    gl_ClipDistance[0] = dot(vsout.worldPosition, gClipPlane.xyz) + gClipPlane.w;
  }

  gl_Position = gViewProjTransform * vec4(vsout.worldPosition, 1.0);

}