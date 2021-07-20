#include "Water.h"
#include <random>
#include "Math.h"
#include <time.h>
#include <chrono>
#include <algorithm>
#include <numeric>

Water::Water(ID3D11Device* device, ID3D11DeviceContext* device_context)
	: TessellationPlaneMesh(device, device_context, MESH_RESOLUTION), amplitude(AMPLITUDE_CONSTANT), wind_speed(WIND_SPEED)
{
	vertexBuffer = NULL;
	device_copy = device;
	device_context_copy = device_context;
	current_buffer = NULL;
	vertexCount = (MESH_RESOLUTION + 1) * (MESH_RESOLUTION + 1);
	indexCount = ((MESH_RESOLUTION) * (MESH_RESOLUTION) * 6);

	//Create vertices and indicies for water object
	vertices = new VertexType[vertexCount];
	complex_vertices = new Vertex[vertexCount];
	indices = new unsigned long[indexCount];

	//Complex numbers for FFT calculations
	for (int i = 0; i < 5; i++)
	{
		NEW_COMPLEX(h[i], MESH_RESOLUTION);
	}

	//Use computer time to generate random numbers
	srand(time(NULL));

	//Gnerate water
	Generate();

	//Complex numbers to store output values from butterfly
	for (int i = 0; i < sizeof(complex_n) / sizeof(complex_n[0]); i++)
		NEW_COMPLEX(complex_n[i], MESH_RESOLUTION);

	//Reverse bits (required for the DFTs in butterfly)
	int n = 1 << (int)std::log2(MESH_RESOLUTION);
	for (int i = 0; i < n; i++) bits_reverse[i] = Math::ReverseBits(i, std::log2(MESH_RESOLUTION));

	n = 2;

	//Root of unity also known as twiddle https://en.wikipedia.org/wiki/Root_of_unity
	Unity_Root = new Math::complex* [std::log2(MESH_RESOLUTION)];

	for (int i = 0; i < (int)std::log2(MESH_RESOLUTION); i++)
	{
		//Roots of unity to be used in DFTs
		Unity_Root[i] = new Math::complex[n];
		for (int j = 0; j < n / 2; j++)
		{
			Unity_Root[i][j] = Math::complex(cos(PI2 * j / n), sin(PI2 * j / n));
		}
		n *= 2;
	}
}

void Water::Generate()
{
	Math::complex h0[2];

	float u, v, increment;
	u = 0; v = 0;
	increment = m_UVscale / (MESH_RESOLUTION + 1);

	indexCount = 0;
	Math::complex noise;

	int index = 0;
	for (int j = 0; j < (MESH_RESOLUTION+ 1); j++)
	{
		for (int i = 0; i < (MESH_RESOLUTION + 1); i++)
		{
			complex_vertices[index].original_normal = XMFLOAT3(0, 1, 0);
			index = j * (MESH_RESOLUTION + 1) + i;

			//Noise adds randomness to the fft making waves more believeable
			noise = Math::Noise(1.f);

			//Stores values of h0 from equation 25 (Simulation Ocean Water, Jerry Tessendorf, 2001)
			h0[0] = noise * (1/sqrt(2)) * sqrt(phillips(i, j));
			h0[1] = Math::Conjugate((noise * (1 / sqrt(2)) * sqrt(phillips(-i, -j))));

			complex_vertices[index].fft_pos.x = h0[0].x;
			complex_vertices[index].fft_pos.y = h0[0].y;
			complex_vertices[index].ifft_pos.x = h0[1].x;
			complex_vertices[index].ifft_pos.y = h0[1].y;

			//Store original vertices, these will be used to calculate the current position of the vertex
			complex_vertices[index].original_pos.x = ((i - MESH_RESOLUTION / 2.0f));
			complex_vertices[index].original_pos.y = ((j - MESH_RESOLUTION / 2.0f));
			complex_vertices[index].pos.x = complex_vertices[index].original_pos.x;
			complex_vertices[index].pos.y = complex_vertices[index].original_pos.y;

			//Set real world vertices to those represented by the complex numbers
			vertices[index].position = XMFLOAT3(complex_vertices[index].pos.x, complex_vertices[index].pos.y, complex_vertices[index].pos.z);
			
			//Vertex originally represent a flat plane for normals are directly upwards
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);

			//For tiled textures
			vertices[index].texture = XMFLOAT2(u, v);

			//2 triangles used to represent the geometry
			if (i < (MESH_RESOLUTION) && j < (MESH_RESOLUTION))
			{
				indices[indexCount++] = index;
				indices[indexCount++] = index + 1;
				indices[indexCount++] = index + MESH_RESOLUTION + 2;
				indices[indexCount++] = index;
				indices[indexCount++] = index + MESH_RESOLUTION + 2;
				indices[indexCount++] = index + MESH_RESOLUTION + 1;
			}
			
			u += increment;
		}
		u = 0;
		v += increment;
	}
	Regenerate();
}

void Water::Regenerate()
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	if (vertexBuffer == NULL) {
		CreateBuffers(device_copy, vertices, indices);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		device_context_copy->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, vertices, sizeof(VertexType) * (vertexCount));
		device_context_copy->Unmap(vertexBuffer, 0);
	}
}

void Water::CreateBuffers(ID3D11Device* device, VertexType* vertices, unsigned long* indices) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * (vertexCount);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (indexCount);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
}

Water::~Water()
{
	//Release
	SAFE_DELETE_ARRAY(complex_vertices);
	for (int i = 0; i < 5; i++)
	{
		SAFE_DELETE_ARRAY(h[i]);
	}
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
}

void Water::Butterfly()
{
	//Butterfly method for FFT https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm
	int sample = 1.f, offset = MESH_RESOLUTION;
	for (int m = 0; m < 2; m++)
	{
		for (int s = 0; s < MESH_RESOLUTION; s++)
		{
			for (int v = 0; v < sizeof(h) / sizeof(h[0]); v++)
			{
				for (int i = 0; i < MESH_RESOLUTION; i++)
					complex_n[current_buffer][i] = h[v][bits_reverse[i] * sample + (offset*s)];

				//Split into 2s so half resoltuion
				int max = MESH_RESOLUTION / 2;
				int step = 2;
				int x = 0;
				int buffer;

				int b = 0;
				//Math::complex even = h[std::slice(0, MESH_RESOLUTION / 2, 2)];

				//External loop
				for (int i = 0; i <= (int)std::log2(MESH_RESOLUTION) - 1; i++)
				{
					//Change current buffer each loop
					(current_buffer == 0 ? current_buffer = 1 : current_buffer = 0);

					//Internal loops
					for (int j = 0; j < max; j++) {
						//DFTs split into 2
						for (int k = 0; k < step / 2; k++) {
							complex_n[current_buffer][step * j + k] = complex_n[abs(current_buffer - 1)][step * j + k] + complex_n[abs(current_buffer - 1)][step * j + step / 2 + k] * Unity_Root[x][k];
						}

						for (int k = step / 2; k < step; k++) {
							complex_n[current_buffer][step * j + k] = complex_n[abs(current_buffer - 1)][step * j - step / 2 + k] - complex_n[abs(current_buffer - 1)][step * j + k] * Unity_Root[x][k - step / 2];
						}
					}
					//Next DFT
					max /= 2;
					step *= 2;
					x++;
				}

				//get output from butterfly
				for (int i = 0; i < MESH_RESOLUTION; i++)
				{
					h[v][i * sample + (offset * s)] = complex_n[current_buffer][i];
				}
			}
		}
		std::swap(offset, sample);
	}
}

//Mathematical equation for the dispersion of water equation 35 (Simulating Ocean Water, Jerry Tessendorf, 2001)
float Water::dispersion(int i, int j)
{
	float w0 = PI2 / m_T; //equation 34
	XMFLOAT2 k = XMFLOAT2((PI * (2 * i) - PI * MESH_RESOLUTION) / MESH_RESOLUTION, (PI * (2 * j) - PI * MESH_RESOLUTION) / MESH_RESOLUTION);
	float w = (sqrt(GRAVITY * sqrt(pow(k.x, 2) + pow(k.y, 2))) / w0) * w0;
	return w;
}

//Perform the FFT calculation on each vertex in the primary water tile 
//These values will be reused for other tiles to improve performance
void Water::FFTWater(float time, float lambda, float T)
{
	m_lambda = lambda;
	m_T = 500.f;
	float hypot;
	XMFLOAT2 k;
	int index, index2, index3;

	//for every vertex
	for (int j = 0; j < MESH_RESOLUTION; j++) {
		k.y = ((PI2 * j) - (PI * MESH_RESOLUTION)) / MESH_RESOLUTION;
		for (int i = 0; i < MESH_RESOLUTION; i++) {
			k.x = ((PI2 * i) - (PI * MESH_RESOLUTION)) / MESH_RESOLUTION;

			//Indexes allow the program to access each individual vertex in the tile
			index = j * MESH_RESOLUTION + i;
			int index2 = j * (MESH_RESOLUTION + 1) + i;

			//h0 stores the positions of vertices affected by fft
			Math::complex h0[2];
			h0[0] = Math::complex(complex_vertices[index2].fft_pos.x, complex_vertices[index2].fft_pos.y);
			h0[1] = Math::complex(complex_vertices[index2].ifft_pos.x, complex_vertices[index2].ifft_pos.y);

			//Eulers formula
			XMFLOAT2 Eulers = XMFLOAT2(cos(dispersion(i, j) * time), sin(dispersion(i, j) * time));

			h[0][index] = h0[0] * Math::complex(Eulers.x, Eulers.y) + h0[1] * Math::complex(Eulers.x, -Eulers.y);
			h[3][index] = h[0][index] * Math::complex(0, k.x);
			h[4][index] = h[0][index] * Math::complex(0, k.y);

			//Get distance between kx and kz
			hypot = Math::Pythagoras(k);
			h[1][index] = h[0][index] * Math::complex(0, -k.x / hypot);
			h[2][index] = h[0][index] * Math::complex(0, -k.y / hypot);

			//Don't let h be negative (epsilon required due to floating point inaccuracy)
			if (hypot < EPSILON) {
				h[1][index] = Math::complex(0.0f, 0.0f);
				h[2][index] = Math::complex(0.0f, 0.0f);
			}

		}
	}

	//Data is now read to enter the butterfly section
	Butterfly();

	//For every vertex
	for (int j = 0; j < MESH_RESOLUTION; j++) {
		for (int i = 0; i < MESH_RESOLUTION; i++) 
		{
			index = j * MESH_RESOLUTION + i;		
			index2 = j * (MESH_RESOLUTION + 1) + i;
			index3 = (MESH_RESOLUTION + 1) * MESH_RESOLUTION;

			//Transform vertices depending on whether they are odd or even 
			//this reduces performance costs as less calculations are required since the first half is equal to
			//the negative of the second half
			//If current working on an odd number invert
			for (int k = 0; k < 5; k++)
				if ((i + j) % 2 != 1)
					h[k][index] = -h[k][index];

			//Update the vertex position using the original position and lamba (dictates the 'choppiness' of waves)
			complex_vertices[index2].pos.x = complex_vertices[index2].original_pos.x + h[1][index].x * lambda;
			complex_vertices[index2].pos.y = h[0][index].x;
			complex_vertices[index2].pos.z = complex_vertices[index2].original_pos.y + h[2][index].x * lambda;

			//Update the normals of each vertex
			n = Math::VecUnit(XMFLOAT3(complex_vertices[index].original_normal.x - h[3][index].x, complex_vertices[index].original_normal.y,
				complex_vertices[index].original_normal.z - h[4][index].x));
			complex_vertices[index2].normal.x = n.x;
			complex_vertices[index2].normal.y = n.y;
			complex_vertices[index2].normal.z = n.z;

			vertices[index2].position = XMFLOAT3(complex_vertices[index2].pos.x, complex_vertices[index2].pos.y, complex_vertices[index2].pos.z);
			vertices[index2].normal = XMFLOAT3(complex_vertices[index2].normal.x, complex_vertices[index2].normal.y, complex_vertices[index2].normal.z);

			//In order to tile seamlessly take the 'extra' vertices (aka MESH_RESOLUTION + 1) and set them to the values of the start of the grid
			//where i == 0 and/or j == 0
			Corners(i, j);
		}
	}
}

float Water::getJacobian(int index)
{
	return complex_vertices[index].jacobian;
}

void Water::Corners(int i, int j)
{
	//index used to get the correct vertex to manipulate
	int index = j * MESH_RESOLUTION;
	int index2 = j * (MESH_RESOLUTION + 1) + i + 1;
	int index3 = (MESH_RESOLUTION + 1) * (j + 1);

	//Tile the top right corner of the tile
	if (i == MESH_RESOLUTION - 1) {
		if (j == MESH_RESOLUTION - 1)
		{
			int index4 = (MESH_RESOLUTION + (MESH_RESOLUTION + 1) * MESH_RESOLUTION);

			complex_vertices[index4].pos.x = complex_vertices[index4].original_pos.x + h[1][0].x * m_lambda;
			complex_vertices[index4].pos.y = h[0][0].x;
			complex_vertices[index4].pos.z = complex_vertices[index4].original_pos.y + h[2][0].x * m_lambda;
			vertices[index4].position = XMFLOAT3(complex_vertices[index4].pos.x, complex_vertices[index4].pos.y, complex_vertices[index4].pos.z);

			complex_vertices[index4].normal.x = n.x;
			complex_vertices[index4].normal.y = n.y;
			complex_vertices[index4].normal.z = n.z;

			vertices[index4].normal = XMFLOAT3(complex_vertices[index4].normal.x, complex_vertices[index4].normal.y, complex_vertices[index4].normal.z);
		}
		//Tile along the x axis where i = MESH_RESOLUTION + 1
			complex_vertices[index2].pos.x = complex_vertices[index2].original_pos.x + h[1][index].x * m_lambda;
			complex_vertices[index2].pos.y = h[0][index].x;
			complex_vertices[index2].pos.z = complex_vertices[index2].original_pos.y + h[2][index].x * m_lambda;
			vertices[index2].position = XMFLOAT3(complex_vertices[index2].pos.x, complex_vertices[index2].pos.y, complex_vertices[index2].pos.z);

			complex_vertices[index2].normal.x = n.x;
			complex_vertices[index2].normal.y = n.y;
			complex_vertices[index2].normal.z = n.z;

			vertices[index2].normal = XMFLOAT3(complex_vertices[index2].normal.x, complex_vertices[index2].normal.y, complex_vertices[index2].normal.z);		
	}
	//Tile along the y axis where j = MESH_RESOLUTION + 1
	if (j == MESH_RESOLUTION-1) {
		complex_vertices[index3 + i].pos.x = complex_vertices[index3 + i].original_pos.x + h[1][i].x * m_lambda;
		complex_vertices[index3 + i].pos.y = h[0][i].x;
		complex_vertices[index3 + i].pos.z = complex_vertices[index3 + i].original_pos.y + h[2][i].x * m_lambda;
		vertices[index3 + i].position = XMFLOAT3(complex_vertices[index3 + i].pos.x, complex_vertices[index3 + i].pos.y, complex_vertices[index3 + i].pos.z);

		complex_vertices[index3 + i].normal.x = n.x;
		complex_vertices[index3 + i].normal.y = n.y;
		complex_vertices[index3 + i].normal.z = n.z;

		vertices[index3 + i].normal = XMFLOAT3(complex_vertices[index3 + i].normal.x, complex_vertices[index3 + i].normal.y, complex_vertices[index3 + i].normal.z);
	}
}

//Philips spectrum, see quation equation 35 (Simulating Ocean Water, Jerry Tessendorf, 2001) https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.161.9102&rep=rep1&type=pdf
float Water::phillips(int i, int j)
{
	XMFLOAT2 tmp = XMFLOAT2((PI * (2 * i) - PI * MESH_RESOLUTION) / MESH_RESOLUTION, (PI * (2 * j) - PI * MESH_RESOLUTION) / MESH_RESOLUTION);

	float k = Math::VecLength(tmp);
	float w = Math::VecLength(wind_speed);

	XMFLOAT2 u_K = Math::VecUnit(tmp);
	XMFLOAT2 u_W = Math::VecUnit(wind_speed);

	float kw = (u_K.x * u_W.x + u_K.y * u_W.y);

	float L = pow(Math::VecLength(wind_speed), 2) / GRAVITY;
	float l = L * EPSILON;
	float Philips = amplitude * exp(-1.f / pow(k * L, 2)) / pow(k, 4)* pow(kw, 2) * exp(pow(-k * l, 2));

	return (k < EPSILON ? 0.f : Philips);
}

//DFT - not real time viable
void Water::DFTWater(float time) {
	float lambda = -1.f;
	int index, index2;

	XMFLOAT2 tmp, k;
	float kL, kx;

	for (int j = 0; j < MESH_RESOLUTION; j++) {
		for (int i = 0; i < MESH_RESOLUTION; i++) {
			index = j * (MESH_RESOLUTION + 1) + i;
			displacement_vector = XMFLOAT2(complex_vertices[index].pos.x, complex_vertices[index].pos.z);
			height = Math::complex(0.0f, 0.0f);
			n = XMFLOAT3(0.0f, 0.0f, 0.0f);

			//Calculate the displacement of the vertices
			for (int l = 0; l < MESH_RESOLUTION; l++) {
				k.y = 2.0f * PI * (l - MESH_RESOLUTION / 2.0f) / MESH_RESOLUTION;
				for (int i = 0; i < MESH_RESOLUTION; i++) {
					index2 = l * (MESH_RESOLUTION) + i;
					k.x = 2.0f * PI * (i - MESH_RESOLUTION / 2.0f) / MESH_RESOLUTION;
					tmp = XMFLOAT2(k.x, k.y);

					kL = Math::VecLength(tmp);
					kx = Math::Vec2Dot(tmp, displacement_vector);

					height = height + h[0][index2] * Math::complex(cos(kx), sin(kx));

					n = XMFLOAT3(n.x + (-k.x * (h[0][index2] * Math::complex(cos(kx), sin(kx))).y), n.y + (0.0f), n.z + (-k.y * (h[0][index2] * Math::complex(cos(kx), sin(kx))).y));

					//Calculate the displacement vector, equation 44 (Simulating Ocean Water, Jerry Tessendorf, 2001)
					displacement_vector = XMFLOAT2(displacement_vector.x + (k.x / kL * (h[0][index2] * Math::complex(cos(kx), sin(kx))).y), displacement_vector.y + (k.y / kL * (h[0][index2] * Math::complex(cos(kx), sin(kx))).y));
				}
			}

			//Calculate new position of vertices
			complex_vertices[index].pos.x = complex_vertices[index].original_pos.x + displacement_vector.x * lambda;
			complex_vertices[index].pos.y = height.x;
			complex_vertices[index].pos.z = complex_vertices[index].original_pos.y + displacement_vector.y * lambda;

			//Calculate new normals
			n = Math::VecUnit((XMFLOAT3(complex_vertices[index].original_normal.x - n.x, complex_vertices[index].original_normal.y - n.y, complex_vertices[index].original_normal.z - n.z)));
			complex_vertices[index].normal.x = n.x;
			complex_vertices[index].normal.y = n.y;
			complex_vertices[index].normal.z = n.z;

			//Set real world vertices to those represented in the complex vertices
			vertices[index].position = XMFLOAT3(complex_vertices[index].pos.x, complex_vertices[index].pos.y, complex_vertices[index].pos.z);
			vertices[index].normal = XMFLOAT3(complex_vertices[index].normal.x, complex_vertices[index].normal.y, complex_vertices[index].normal.z);
		}
	}
}

void Water::sendData(ID3D11DeviceContext* device_context)
{
	TessellationPlaneMesh::sendData(device_context);
	BaseMesh::sendData(device_context);
}

void Water::render(float t, float lambda, float T)
{
	//Simulate FFT water and render the water tile(s)
	FFTWater(t, lambda, T);
	Regenerate();
}

void Water::useFFT(Water* cpy, float offsetX, float offsetZ)
{
	//If using tiles set each vertex to the corresponding vertex
	//on the original tile; this improves performance
	//as fft only has to be calcualted once
	int index;
	for (int j = 0; j < MESH_RESOLUTION + 1; j++) {
		for (int i = 0; i < MESH_RESOLUTION + 1; i++) 
		{
			index = j * (MESH_RESOLUTION + 1) + i;

			cpy->vertices[index].position = vertices[index].position;
			cpy->vertices[index].position.x = vertices[index].position.x + offsetX;
			cpy->vertices[index].position.z = vertices[index].position.z + offsetZ;
			cpy->vertices[index].normal = vertices[index].normal;
		}
	}
	cpy->Regenerate();
}