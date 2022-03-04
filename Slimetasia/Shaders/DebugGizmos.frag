#version 410 core

uniform int gizmoIndex;
uniform vec4 color;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outIndex;

void main()
{
  outColor = color;
  outIndex = gizmoIndex;
}