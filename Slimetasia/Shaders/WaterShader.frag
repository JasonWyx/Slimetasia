#version 450 core

uniform sampler2D uReflectionTex;
uniform sampler2D uRefractionTex;
uniform sampler2D uDepthTex;
uniform sampler2D uDUDVTex;
uniform sampler2D uDUDVNorm;
uniform float     uFresnelPower;
uniform vec3      uWaterColor;
uniform float     uWaveFactor;
uniform float     uWaveStrength;
uniform float     uWaterDepth;
uniform float     uCamNear;
uniform float     uCamFar;

layout(location = 0) in vec4 iPositionCS;
layout(location = 1) in vec2 iTexCoords;
layout(location = 2) in vec3 iViewVector;

layout(location = 0) out vec4 oFinalColor;

vec2 ProjectiveTexCoords(vec4 positionCS)
{
  // Shift from (-1, 1) to (0, 1)
  vec2 ndc = (positionCS.xy / positionCS.w) / 2.0 + 0.5;
  return ndc;
}

void main()
{
  vec2 refractTexCoords = ProjectiveTexCoords(iPositionCS);
  vec2 reflectTexCoords = vec2(refractTexCoords.x, -refractTexCoords.y);

  float depth = texture(uDepthTex, refractTexCoords).r;
  float floorDist = 2.0 * uCamNear * uCamFar / (uCamFar + uCamNear - (2.0 * depth - 1.0) *  (uCamFar - uCamNear));

  depth = gl_FragCoord.z;
  float waterDist = 2.0 * uCamNear * uCamFar / (uCamFar + uCamNear - (2.0 * depth - 1.0) *  (uCamFar - uCamNear));
  float waterDepth = floorDist - waterDist;

  vec2 distort1 = (texture(uDUDVTex, vec2(iTexCoords.x + uWaveFactor, iTexCoords.y)).rg * 2.0 - 1.0) * uWaveStrength;
  vec2 distort2 = (texture(uDUDVTex, vec2(-iTexCoords.x + uWaveFactor, iTexCoords.y + uWaveFactor)).rg * 2.0 - 1.0) * uWaveStrength * clamp(waterDepth/uWaterDepth, 0.0, 1.0);
  vec2 totalDistort = distort1 + distort2;

  refractTexCoords += totalDistort;
  refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);

  reflectTexCoords += totalDistort;
  reflectTexCoords.x = clamp(reflectTexCoords.x, 0.001, 0.999);
  reflectTexCoords.y = clamp(reflectTexCoords.y, -0.999, -0.001);

  //vec2 refractTexCoords = ProjectiveTexCoords(iPositionCS);
  //vec2 reflectTexCoords = vec2(refractTexCoords.x, -refractTexCoords.y);

  vec4 reflectColor = texture(uReflectionTex, reflectTexCoords);
  vec4 refractColor = texture(uRefractionTex, refractTexCoords);

  float refractFactor = dot(normalize(iViewVector), vec3(0.0, 1.0, 0.0));
  refractFactor = pow(refractFactor, uFresnelPower);

  oFinalColor = mix(reflectColor, refractColor, refractFactor);
  oFinalColor = mix(oFinalColor, vec4(uWaterColor, 1.0), 0.5);
  //oFinalColor = texture(uDUDVTex, iTexCoords);

  oFinalColor.a = clamp(waterDepth/uWaterDepth, 0.0, 1.0);
}