#version 410 core

uniform sampler2D gRenderTexture;
uniform ivec2     gScreenSize;
uniform int       gPPFlags;

uniform sampler2D gBloomTexture;

out vec4 outColor;

vec2 CalculateTexCoord()
{
  return gl_FragCoord.xy / gScreenSize;
}

void main()
{
  vec2 texCoord = CalculateTexCoord();
  vec4 fragColor = texture(gRenderTexture, texCoord);

  outColor = fragColor;

  if((gPPFlags & PPFLAG_BLOOM) != 0)
  {
    outColor += texture(gBloomTexture, texCoord);
  }

}