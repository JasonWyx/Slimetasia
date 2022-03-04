#version 410 core

uniform sampler2D gTexture;
uniform ivec2     gScreenSize;

out vec4 outColor;

vec2 CalculateTexCoord()
{
  return gl_FragCoord.xy / gScreenSize;
}

void main()
{
  vec4 texColor = texture(gTexture, CalculateTexCoord());
  float brightness = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));

  if(brightness > 1.0)
    outColor = texColor;
  else
    outColor = vec4(0.0);
}