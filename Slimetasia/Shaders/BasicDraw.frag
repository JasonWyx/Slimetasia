#version 410 core

layout(location = 0) in vec2 iTexCoords;

uniform sampler2D gTexture;

out vec4 outColor;

void main()
{
  outColor = texture(gTexture, iTexCoords);
}