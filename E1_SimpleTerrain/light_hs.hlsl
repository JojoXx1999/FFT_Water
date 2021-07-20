// Tessellation Hull Shader
// Prepares control points for tessellation
struct InputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;

	// Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = 2;
    output.edges[1] = 2;
    output.edges[2] = 2;

	// Set the tessellation factor for tessallating inside the triangle.
    output.inside = 2;

    return output;
}


[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;


	// Set the position for this control point as the output position.
    output.position = patch[pointId].position;

	// Set the input colour as the output colour.
    output.colour = patch[pointId].colour;



    return output;
}