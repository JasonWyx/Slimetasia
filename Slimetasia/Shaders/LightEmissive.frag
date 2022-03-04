#version 410 core

uniform sampler2D uEmissiveTex;

layout(location = 0) in vec2 iTexCoords;

out vec4 outColor;

void main()
{
  vec3 emissiveColor = texture(uEmissiveTex, iTexCoords).rgb;

  if(length(emissiveColor) == 0.0) discard;

  outColor = vec4(emissiveColor, 1.0);
}