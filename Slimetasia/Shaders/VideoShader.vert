#version 450 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexCoords;

uniform mat4  gMTransform;
uniform mat4  gVPTransform;

layout(location = 0) out vec2 oTexCoords;
layout(location = 1) out vec3 oWorldPosition;

void main()
{
  oTexCoords      = iTexCoords;
  oWorldPosition  = (gMTransform * vec4(iPosition, 1.0)).xyz; 
  gl_Position     = gVPTransform * vec4(oWorldPosition, 1.0);
}