#version 450 core

layout(location = 0) in vec2 iTexCoords;
layout(location = 1) in vec3 iWorldPosition;

layout(binding = 0) uniform sampler2D gFontAtlas;

uniform vec4  gFontColor;
uniform float gSDF;
uniform vec3  gWorldNormal;

uniform bool  gOutlineEnabled;
uniform float gOutlineSDF;
uniform vec4  gOutlineColor;

uniform int   gObjectID;

layout(location = 0) out vec4 oFinalColor;
layout(location = 3) out vec3 oWorldNormal;
layout(location = 4) out vec3 oWorldPosition;
layout(location = 6) out int  oObjectID;

const float smoothing = 1.0 / 16.0;

void main()
{
  float distance = texture(gFontAtlas, iTexCoords).r;
  float alpha = smoothstep(gSDF - smoothing, gSDF + smoothing, distance);
  float outline = smoothstep(gSDF - smoothing, gSDF + smoothing, distance);

  if(distance > gSDF)
  {
    if(gOutlineEnabled && distance < gOutlineSDF)
    {
      oFinalColor = vec4(gOutlineColor.rgb, gOutlineColor.a * alpha);
    }
    else
    {
      oFinalColor = vec4(gFontColor.rgb, gFontColor.a * alpha);
    }

    oWorldPosition  = iWorldPosition;
    oWorldNormal    = gWorldNormal;
    oObjectID       = gObjectID;
  }
  else
  {
    discard;
  }
}