#version 410 core

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in float inSize;
layout (location = 3) in float inFade;


uniform mat4 gViewTransform;

out VS_OUT
{
  float size;
  float fade;
  vec4 color; 
} vsOut;


void main()
{
  vsOut.size = inSize;
  gl_Position = gViewTransform * inPosition;
  vsOut.color = inColor;
  vsOut.fade = inFade;
}