#version 410 core

layout(location = 0) in vec2 iTexCoords;

uniform sampler2D gSSAOInput;

layout(location = 0) out float oBlurredValue;

void main()
{
  vec2 texelSize = 1.0 / vec2(textureSize(gSSAOInput, 0));
  float result = 0.0;

  for(int x = -2; x < 2; ++x)
  {
    for(int y = -2; y < 2; ++y)
    {
      vec2 offset = vec2(x, y) * texelSize;
      result += texture(gSSAOInput, iTexCoords + offset).r;
    }
  }

  oBlurredValue = result / (4.0 * 4.0);
}