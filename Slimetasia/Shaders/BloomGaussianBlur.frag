#version 410 core

layout(location = 0) in vec2 iTexCoords;

uniform sampler2D gTexture;
uniform ivec2     gScreenSize;

// Gaussian blur attributes
uniform bool      gHorizontal;
uniform float     weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec4 outColor;

vec2 CalculateTexCoord()
{
  return gl_FragCoord.xy / gScreenSize;
}

vec2 CalculateTexCoord(vec2 offset)
{
  return (gl_FragCoord.xy + offset) / gScreenSize;
}

void main()
{
  vec2 texCoord = CalculateTexCoord();
  vec4 texColor = texture(gTexture, texCoord);
  vec4 result = texColor * weight[0];

  if(gHorizontal)
  {
    for(int i = 1; i < 5; ++i)
    {
      result += texture(gTexture, CalculateTexCoord(vec2(i, 0.0)) ) * weight[i];
      result += texture(gTexture, CalculateTexCoord(vec2(-i, 0.0)) ) * weight[i];
    }
  }
  else
  {
    for(int i = 1; i < 5; ++i)
    {
      result += texture(gTexture, CalculateTexCoord(vec2(0.0, i)) ) * weight[i];
      result += texture(gTexture, CalculateTexCoord(vec2(0.0, -i)) ) * weight[i];
    }
  }

  outColor = result;

}