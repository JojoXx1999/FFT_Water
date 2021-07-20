#pragma once
#include "complex.h"
#include "Math.h"
#include "Macros.h"
#include "TessellationPlaneMesh.h"

struct Vertex
{
	XMFLOAT2 original_pos;
	XMFLOAT3 pos;
	XMFLOAT3 original_normal;
	XMFLOAT3 normal;
	XMFLOAT2 fft_pos;
	XMFLOAT2 ifft_pos;
	float jacobian;
};

struct FrustumPoints
{
	static const unsigned number_camera_edges{ 12 };

	const int camera_edges[number_camera_edges * 2]
	{
		0, 1,
		1, 3,
		3, 2,
		2, 0,

		0, 4,
		1, 5,
		3, 7,
		2, 6,

		4, 5,
		5, 7,
		7, 6,
		6, 4
	};
};

class Water : public TessellationPlaneMesh
{
public:
	Water(ID3D11Device* device, ID3D11DeviceContext* device_context);
	~Water();

	//Dispersion for philips spectrum equation 35 (Simulating Ocean Water, Jerry Tessendorf, 2001) https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.161.9102&rep=rep1&type=pdf
	float dispersion(int i, int j);

	//Philips spectrum equation 23(Simulating Ocean Water, Jerry Tessendorf, 2001)
	float phillips(int i, int j);

	//Apply FFT
	void FFTWater(float t, float lambda, float T);

	//Apply DFT
	void DFTWater(float t);

	//Run simulation
	void render(float t, float lambda, float T);

	//return jacobian
	float getJacobian(int index);

	//T for dispersion
	float m_T;

	//Set amplitude
	inline void Amplitude(float Amplitude) { UPDATE_VARIABLE(amplitude, Amplitude); }

	//Return amplitude
	float Amplitude() { return amplitude; }

	//Set wind speed
	inline void WindSpeed(XMFLOAT2 ws) { UPDATE_VARIABLE(wind_speed.x, ws.x);  UPDATE_VARIABLE(wind_speed.y, ws.y); }

	//Return wind speed
	XMFLOAT2 WindSpeed() { return wind_speed; }

	void sendData(ID3D11DeviceContext* device_context);

	//Generate the water plane
	void Generate();

	//Apply the fft to tiled water without carrying out extra calculations
	void useFFT(Water* cpy, float offsetX, float offsetY);

	//Set vertex position
	inline void setVertexPos(XMFLOAT3 pos, int index) { vertices[index].position = pos; }

private:
	void CreateBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices);

	//Regenerate the water object
	void Regenerate();

	//Perform butterfly fft
	void Butterfly();

	void Corners(int i, int j);

	//Philips spectrum variables
	float amplitude;
	XMFLOAT2 wind_speed;

	XMFLOAT2 displacement_vector; //displacement

	//Complex numbers for FFT based on equations provided by Jerry Tessendorf(Simulating Ocean Water, Jerry Tessendorf, 2001)
	Math::complex* h[5];
	Math::complex height; //wave height

	//Real world vertices for displaying water
	VertexType* vertices;

	//vertices for performing calculations
	Vertex* complex_vertices;

	//water indicies
	unsigned long* indices;

	//vertex buffer objects
	int vbo_vertices, vbo_indices;

	//Tile the UV map 20 times across the plane
	const float m_UVscale = 20.0f;	

	ID3D11Device* device_copy;
	ID3D11DeviceContext* device_context_copy;

	XMFLOAT3 n; //normal

	int current_buffer;
	int bits_reverse[MESH_RESOLUTION];

	Math::complex** Unity_Root;
	Math::complex* complex_n[2];

	float m_lambda;
};