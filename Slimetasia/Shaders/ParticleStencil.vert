#version 410 core

layout (location = 0) in vec4 inPosition;
layout (location = 2) in float inSize;

out VS_OUT
{
  float size;
} vsOut;


uniform mat4 gViewTransform;

void main()
{
  gl_Position = gViewTransform * inPosition;
  vsOut.size = inSize;
}