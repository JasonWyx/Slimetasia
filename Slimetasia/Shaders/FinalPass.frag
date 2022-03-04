#version 410 core

layout(location = 0) in vec2 iTexCoords;

uniform sampler2D gRenderTexture;

uniform float     gGamma;
uniform float     gExposure;

out vec4 outColor;


void main()
{
  vec4 hdrColor = texture(gRenderTexture, iTexCoords);

  vec3 mappedColor = vec3(1.0) - exp(-hdrColor.rgb * gExposure);
       mappedColor = pow(mappedColor, vec3(1.0 / gGamma));

  outColor = vec4(mappedColor, min(hdrColor.a, 1.0));
}