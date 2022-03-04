#version 410 core

uniform sampler2D gDiffuseTexture;
uniform sampler2D gNormalTexture;
uniform sampler2D gSpecularTexture;
uniform float     gSpecularShininess;
uniform bool      gEmissiveEnabled;
uniform sampler2D gEmissiveTexture;
uniform vec3      gEmissiveColor;

uniform vec4  gMeshColor;
uniform bvec4 gTextureFlags; // Albedo, Normal, Specular, Emissive
uniform int   gPickingId; 

in VertData
{
  vec3 worldPosition;
  vec3 worldNormal;
  vec3 worldTangent;
  vec3 worldBitangent;
  vec3 vertColor;
  vec2 texCoord;

} vsout;

layout(location = 0) out vec4 outDiffuse;
layout(location = 1) out vec4 outSpecular;
layout(location = 2) out vec3 outEmissive;
layout(location = 3) out vec3 outWorldNormal;
layout(location = 4) out vec3 outWorldPosition;
layout(location = 5) out vec2 outTexCoords;
layout(location = 6) out int  outIndex;

void main()
{
  vec4 texColor = texture(gDiffuseTexture, vsout.texCoord);

  // Discard if alpha of either is 0
  if(texColor.a <= 0.5 || gMeshColor.a <= 0.0) discard;

  mat3 TBN = mat3(
    normalize(vsout.worldTangent), 
    normalize(vsout.worldBitangent),
    normalize(vsout.worldNormal)
  );

  outDiffuse        = (gTextureFlags[0] == true ? texColor : vec4(1.0)) * gMeshColor;
  outSpecular       =  gTextureFlags[2] == true ? vec4(texture(gSpecularTexture, vsout.texCoord).rgb, gSpecularShininess) : vec4(0.0);
  outEmissive       =  gEmissiveEnabled ? (gTextureFlags[3] == true ? texture(gEmissiveTexture, vsout.texCoord).rgb * gEmissiveColor : gEmissiveColor) : vec3(0.0);
  outWorldNormal    =  normalize(gTextureFlags[1] == true ? TBN * (texture(gNormalTexture, vsout.texCoord).xyz * 2.0 - vec3(1.0)) : vsout.worldNormal);
  outWorldPosition  =  vsout.worldPosition;
  outTexCoords      =  vsout.texCoord;
  outIndex          =  gPickingId;
}