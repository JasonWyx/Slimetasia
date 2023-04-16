

struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR0;
};

VSOutput main(uint vertexId : SV_VertexID)
{
	VSOutput output = (VSOutput)0;

	float4 positions[3] = { { 0.0f, -0.5f, 0.0f, 1.0f }, { -0.5f, 0.5f, 0.0f, 1.0f }, { 0.5f, 0.5f, 0.0f, 1.0f } };
    float3 colors[3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

	output.Position = positions[vertexId];
    output.Color = colors[vertexId];

	return output;
}