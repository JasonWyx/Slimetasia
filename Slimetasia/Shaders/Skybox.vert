#version 410 core

layout(location = 0) in vec3 position;

uniform mat4 viewProjection;

out vec3 texCoords;

void main()
{
  texCoords = position;
  vec4 pos = viewProjection * vec4(position, 1.0);
  gl_Position = pos.xyww;
}