#version 410 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 gProjTransform;

in VS_OUT
{
  float size;
  float fade;
  vec4 color;  
} gsIn[];

out GS_OUT
{
  vec4 color;
  vec2 texCoords;
  float fade;
} gsOut;


void main()
{
  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(-0.5 * gsIn[0].size, -0.5 * gsIn[0].size, 0, 0));
  gsOut.color     = gsIn[0].color;
  gsOut.fade      = gsIn[0].fade;
  gsOut.texCoords = vec2(0.0, 0.0);
  EmitVertex();

  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(0.5 * gsIn[0].size, -0.5 * gsIn[0].size, 0, 0));
  gsOut.color     = gsIn[0].color;
  gsOut.fade      = gsIn[0].fade;
  gsOut.texCoords = vec2(1.0, 0.0);
  EmitVertex();

  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(-0.5 * gsIn[0].size, 0.5 * gsIn[0].size, 0, 0));
  gsOut.color     = gsIn[0].color;
  gsOut.fade      = gsIn[0].fade;
  gsOut.texCoords = vec2(0.0, 1.0);
  EmitVertex();

  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(0.5 * gsIn[0].size, 0.5 * gsIn[0].size, 0, 0));
  gsOut.color     = gsIn[0].color;
  gsOut.fade      = gsIn[0].fade;
  gsOut.texCoords = vec2(1.0, 1.0);
  EmitVertex();

  EndPrimitive();
}