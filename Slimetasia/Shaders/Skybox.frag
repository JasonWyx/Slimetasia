#version 410 core

in vec3 texCoords;

uniform samplerCube skyboxTex;
uniform vec4 skyboxColor;

out vec4 outColor;

void main()
{
  outColor = texture(skyboxTex, texCoords) * skyboxColor;
}