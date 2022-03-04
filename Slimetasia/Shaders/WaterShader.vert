#version 450 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexCoords;

uniform mat4 uMTransform;
uniform mat4 uVPTransform;
uniform vec3 uCameraPosition;
uniform float uTilingFactor;

layout(location = 0) out vec4 oPositionCS;
layout(location = 1) out vec2 oTexCoords;
layout(location = 2) out vec3 oViewVector;

void main()
{
  vec3 worldPosition = (uMTransform * vec4(iPosition, 1.0)).xyz;
  oPositionCS = uVPTransform * vec4(worldPosition, 1.0);
  oTexCoords  = iTexCoords * uTilingFactor;
  oViewVector = uCameraPosition - worldPosition;
  gl_Position = oPositionCS;
}