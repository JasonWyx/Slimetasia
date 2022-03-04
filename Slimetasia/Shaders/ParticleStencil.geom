#version 410 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 gProjTransform;

in VS_OUT
{
  float size;
} gsIn[];

void main()
{
  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(-0.5 * gsIn[0].size, -0.5 * gsIn[0].size, 0, 0));
  EmitVertex();

  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(0.5 * gsIn[0].size, -0.5 * gsIn[0].size, 0, 0));
  EmitVertex();

  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(-0.5 * gsIn[0].size, 0.5 * gsIn[0].size, 0, 0));
  EmitVertex();

  gl_Position     = gProjTransform * (gl_in[0].gl_Position + vec4(0.5 * gsIn[0].size, 0.5 * gsIn[0].size, 0, 0));
  EmitVertex();

  EndPrimitive();
}