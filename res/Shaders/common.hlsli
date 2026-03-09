
struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
    float3 NormalL : NORMAL;
    float4 Color : COLOR;
    float3 TangentL : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    float3 TangentW : TANGENT;
};

struct VertexUiIn
{
    float2 PosL : POSITION;
    float2 TexC : TEXCOORD;
    float4 Color : COLOR;
};

struct VertexUiOut
{
    float4 PosH : SV_POSITION;
    //float2 PosW : POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
}

cbuffer cbPerPassData : register(b2)
{
    float4x4 gViewProj;
    float4x4 gView;
    float4x4 gProj;
    float4x4 gInvViewProj;

    float3 gEyePosW;
    float gNearZ;

    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float pad0;

    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
};