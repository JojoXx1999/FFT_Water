#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class LightShader : public BaseShader
{
private:
	//Data to be passed into pixel shader
	struct LightBufferType
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse[2];
		XMFLOAT4 position;
		XMFLOAT4 specularColor[2];
		XMFLOAT4 WaterColor[3];
		XMFLOAT4 LightAtt1;
		XMFLOAT4 LightAtt2;
		XMFLOAT4 LightAtt3;
		XMFLOAT4 direction[2];
	};

	//Camera data for shaders
	struct CameraBufferType
	{
		XMFLOAT3 camera_position;
		float padding;
	};

public:
	LightShader(ID3D11Device* device, HWND hwnd);
	~LightShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* texture2, Light* light, Light* point_light,
		XMFLOAT3 deepWater, XMFLOAT3 shallowWater, XMFLOAT3 atmopshereWater, float reflectivity, float reflectivityDirect, float reflectivitySpot, float foam, float ID, float ID2, XMFLOAT3 camera_position, float minHeight, float maxHeight);

private:
	//Load required shaders in the pipeline
	void initShader(const wchar_t* cs, const wchar_t* hs, const wchar_t* ds, const wchar_t* ps);
	void initShader(const wchar_t* cs, const wchar_t* ps);
private:
	//Buffers for sending dara
	ID3D11Buffer * matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer, *cameraBuffer;
};

