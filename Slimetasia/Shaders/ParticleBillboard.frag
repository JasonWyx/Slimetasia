#version 410 core


in GS_OUT
{
  vec4 color;
  vec2 texCoords;
  float fade;
} fragIn;


uniform sampler2D gBillboardTexture;

uniform sampler2D gBillboardEndTexture;

out vec4 fragColor;

void main()
{
  vec4 texColor = texture(gBillboardTexture, fragIn.texCoords);
  vec4 endTexColor = texture(gBillboardEndTexture, fragIn.texCoords);

  float alpha = 1.0;

  if(fragIn.fade >= 1.0) alpha = 1.0;
  else if(fragIn.fade <= 0.0) alpha = 0.0;
  else alpha = fragIn.fade;

  if(texColor.a == 0.0) discard;

  vec4 firstColor = texColor * fragIn.color;
  vec4 secondColor = endTexColor * fragIn.color;

  fragColor = firstColor * alpha + secondColor * (1 - alpha);
}