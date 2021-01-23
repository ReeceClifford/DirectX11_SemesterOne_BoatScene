//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
//Adding Texture to the Shader
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	float gTime;
	matrix World;
	matrix View;
	matrix Projection;

	float pad;
	float3 LightVecW;
	float4 diffuseMtrl;
	float4 diffuseLight;

	float4 ambientMtrl;
	float4 ambientLight;

	float4 SpecularMtrl;
	float4 SpecularLight;
	float SpecularPower;
	float3 EyePosW;

	
}

//--------------------------------------------------------------------------------------
//struct VS_INPUT
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
//struct PS_INPUT
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
//struct VS_OUTPUT
//--------------------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Pos: SV_POSITION;
	float4 Color : COLOR0;
	float3 norm : NORMAL;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float3 Normal : NORMAL, VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.Pos = mul(Pos, World);

	//Apply View and Projection transformations
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	//Convert from local space to world space. W component of vector is 0 as vectors cannot be translated
	float3 normalW = mul(float4(Normal, 0.0f), World).xyz;
	normalW = normalize(normalW);

	output.Tex = input.Tex;
	output.norm = normalW;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);

	//Compute Vector from vertex to the Eye Position
	float3 toEye = normalize(EyePosW - input.Pos.xyz);

	//Compute the reflection Colour
	float3 r = reflect(-LightVecW, input.norm);

	// Determine how much (if any) specular light makes it into the eye.
	float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);

	//Compute Ambient, Diffuse and Specular Lighting
	float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;
	float diffuseAmount = max(dot(LightVecW, input.norm), 0.0f);
	float3 diffuse = (diffuseAmount * (diffuseMtrl * diffuseLight).rgb);
	float3 ambient = (ambientMtrl * ambientLight);

	// Sum of all together + Diffuse Alpha
	input.Color.rgb = textureColour + (ambient + diffuse + specular);
	input.Color.a = diffuseMtrl.a;
    return input.Color;
}
//--------------------------------------------------------------------------------------
// Water Shaders
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Water Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSWATER(float4 Pos : POSITION, float3 Normal : NORMAL, VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	Pos.xy += -1.8f * sin(Pos.x) * sin(5.5f * gTime);
	Normal.y += -1.8f * sin(Normal.xyz) * sin(5.5f * gTime);

	//Apply View and Projection and World transformations
	output.Pos = mul(Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	
	//Convert from local space to world space. W component of vector is 0 as vectors cannot be translated
	float3 normalW = mul(float4(Normal, 0.0f), World).xyz;
	normalW = normalize(normalW);

	output.Tex = input.Tex;
	output.norm = normalW;
	return output;
}
//--------------------------------------------------------------------------------------
// Water Pixel Shader
//--------------------------------------------------------------------------------------
float4 PSWATER(VS_OUTPUT input) : SV_Target
{ 

float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);

//Compute Vector from vertex to the Eye Position
float3 toEye = normalize(EyePosW - input.Pos.xyz);

//Compute the reflection Colour
float3 r = reflect(-LightVecW, input.norm);

// Determine how much (if any) specular light makes it into the eye.
float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);

//Compute Ambient, Diffuse and Specular Lighting
float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;
float diffuseAmount = max(dot(LightVecW, input.norm), 0.0f);
float3 diffuse = (diffuseAmount * (diffuseMtrl * diffuseLight).rgb);
float3 ambient = (ambientMtrl * ambientLight);

// Sum of all together + Diffuse Alpha

input.Color.rgb = textureColour + (ambient + diffuse + specular);
input.Color.a = diffuseMtrl.a;
return input.Color;
}

