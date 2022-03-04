#version 450 core

layout(location = 0) in vec2 iTexCoords;
layout(location = 1) in vec3 iWorldPosition;

layout(binding = 0) uniform sampler2D gFrameTexture;

uniform int   gObjectID;

layout(location = 0) out vec4 oFinalColor;
layout(location = 3) out vec3 oWorldPosition;
layout(location = 4) out int  oObjectID;

void main()
{
  oFinalColor = texture(gFrameTexture, iTexCoords);
  oWorldPosition = iWorldPosition;
  oObjectID = gObjectID;
}