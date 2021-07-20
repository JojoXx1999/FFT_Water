#include "Math.h"

namespace Math {

	unsigned int complex::additions = 0;
	unsigned int complex::multiplications = 0;

	complex::complex() : x(0.0f), y(0.0f) { }
	complex::complex(float x, float y) : x(x), y(y) { }

	//Multiply complex numbers
	complex complex::operator*(const complex& c) const {
		complex::multiplications++;
		return complex(this->x * c.x - this->y * c.y, this->x * c.y + this->y * c.x);
	}

	//Add complex numbers
	complex complex::operator+(const complex& c) const {
		complex::additions++;
		return complex(this->x + c.x, this->y + c.y);
	}

	//Subtract complex numbers
	complex complex::operator-(const complex& c) const {
		complex::additions++;
		return complex(this->x - c.x, this->y - c.y);
	}

	//Subtract
	complex complex::operator-() const {
		return complex(-this->x, -this->y);
	}

	//Multiply
	complex complex::operator*(const float c) const {
		return complex(this->x * c, this->y * c);
	}

	//Equate complex numbers
	complex& complex::operator=(const complex& c) {
		this->x = c.x; this->y = c.y;
		return *this;
	}

	//Generate noise
	complex Noise(float limit)
	{
		float a, b, c;
		do {
			a = ((float)rand() / RAND_MAX)*2 - 1.f;
			b = ((float)rand() / RAND_MAX)*2 - 1.f;
			c = pow(a, 2) + pow(b, 2);
		} while (c >= limit);

		c = sqrt((-2.f * log(c)) / c);
		return Math::complex(a * c, b * c);
	}

	//Vector to point
	float Vec2Dot(const XMFLOAT2& x, const XMFLOAT2& y)
	{
		return (x.x * x.x + y.y * y.y);
	}

	//Normalise vector
	void Vec2Normalize(XMFLOAT2& out, const XMFLOAT2& v)
	{
		float a = 1.f / sqrtf(v.x * v.x + v.y * v.y);	
		out.x = v.x * a;
		out.y = v.y * a;
	}

	//Length of 2D vector
	float VecLength(XMFLOAT2& v)
	{
		return sqrtf(v.x * v.x + v.y * v.y);
	}

	//Length of 3D vector
	float VecLength(XMFLOAT3& v)
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	//Unit of 2D vector
	XMFLOAT2 VecUnit(XMFLOAT2& v)
	{
		float l = VecLength(v);
		return XMFLOAT2(v.x/l, v.y/l);
	}

	float Pythagoras(XMFLOAT2 a)
	{
		return sqrt(pow(a.x, 2) + pow(a.y, 2));
	}

	//Unit of 3D vector
	XMFLOAT3 VecUnit(XMFLOAT3& v)
	{
		float l = VecLength(v);
		return XMFLOAT3(v.x / l, v.y / l, v.z / l);
	}

	//Log2
	uint32_t Log2OfPow2(uint32_t x)
	{
		uint32_t a = 0;

		while (x >>= 1)
			++a;

		return a;
	}

	//Transform point
	XMFLOAT3 transformPoint(const XMMATRIX transform, const XMFLOAT3& point)
	{
		XMFLOAT4X4 tmp;
		XMStoreFloat4x4(&tmp, transform);
		float x = tmp.m[0][0] * point.x + tmp.m[1][0] * point.y + tmp.m[2][0] * point.z + tmp.m[3][0];
		float y = tmp.m[0][1] * point.x + tmp.m[1][1] * point.y + tmp.m[2][1] * point.z + tmp.m[3][1];
		float z = tmp.m[0][2] * point.x + tmp.m[1][2] * point.y + tmp.m[2][2] * point.z + tmp.m[3][2];
		float w = tmp.m[0][3] * point.x + tmp.m[1][3] * point.y + tmp.m[2][3] * point.z + tmp.m[3][3];

		if (w == 1.f)
			return XMFLOAT3(x, y, z);
		else
			return XMFLOAT3(x / w, y / w, z / w);
	}

	//Distance from point to plane
	float distance_from_point_to_plane(const XMFLOAT3& point, const PlaneMesh& plane)
	{
		//return point.x * plane.x + point.y * plane.y + point.z + plane.z;
		return 0;
	}

	//Reverse bits
	uint32_t ReverseBits(uint32_t bits, uint32_t log2)
	{
		uint32_t reverse_num = 0, tmp;

		for (int j = 0; j < log2; j++)
		{
			tmp = (bits & (1 << j));
			if (tmp)
				reverse_num |= (1 << ((log2 - 1) - j));
		}
		return reverse_num;

		return bits;
	}

	//Conjugate complex number
	complex Conjugate(complex c)
	{
		return complex(c.x, -c.y);
	}
}