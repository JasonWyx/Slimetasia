#version 410 core

in vec4 FragPos;

uniform vec3 gLightPosition;
uniform float gShadowDistance; // Far plane of the light view

void main()
{
  // Calculate normalized depth from [0, 1]
  gl_FragDepth = length(FragPos.xyz - gLightPosition) / gShadowDistance;
}