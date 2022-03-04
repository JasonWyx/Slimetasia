#version 410 core

uniform bool      uDiffuseEnabled;
uniform sampler2D uDiffuseTexture;

layout(location = 0) in vec2 inTexCoords;

void main()
{
  if(uDiffuseEnabled && texture(uDiffuseTexture, inTexCoords).a == 0.0)
  {
    discard;
  }
}