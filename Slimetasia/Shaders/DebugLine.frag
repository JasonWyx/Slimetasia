#version 410 core

out vec4 finalColor;

layout(location = 0) in vec4 inColor;

void main()
{
  finalColor = inColor;
}