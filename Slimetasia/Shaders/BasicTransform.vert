#version 410 core

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexCoords;

uniform mat4 gMVP;

layout(location = 0) out vec2 oTexCoords;

void main()
{
  gl_Position = gMVP * vec4(iPosition, 1.0);
  oTexCoords = iTexCoords;
}