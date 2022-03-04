#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 viewProjTransform;
uniform vec4 color;

layout(location = 0) out vec4 outColor;

void main()
{
  gl_Position = viewProjTransform * vec4(position, 1.0);
  outColor = color;
}