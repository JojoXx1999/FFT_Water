#pragma once
#include "DXF.h"

using namespace DirectX;

namespace Math {

    //Create complex number
	struct complex
	{
		float x, y;
        static unsigned int additions, multiplications;
        complex();
        complex(float x, float y);

        //Opperators for complex type
        complex operator*(const complex& c) const;
        complex operator+(const complex& c) const;
        complex operator-(const complex& c) const;
        complex operator-() const;
        complex operator*(const float c) const;
        complex& operator=(const complex& c);
	};

  
		float Vec2Dot(const XMFLOAT2& a, const XMFLOAT2& b); //Vector to point
		void Vec2Normalize(XMFLOAT2& out, const XMFLOAT2& v); //Normalise vector
		float VecLength( XMFLOAT2& v); //Length of the 2D vector
        float VecLength( XMFLOAT3& v); //Length of the 3D vector
        XMFLOAT2 VecUnit( XMFLOAT2& v); //Length of the 2D unit
        XMFLOAT3 VecUnit( XMFLOAT3& v); //Length of the 3d unit
		uint32_t Log2OfPow2(uint32_t x); //Log2
        XMFLOAT3 transformPoint(const XMMATRIX transform, const XMFLOAT3& point); //Transform point
        float Pythagoras(XMFLOAT2);
        float distance_from_point_to_plane(const XMFLOAT3& point, const PlaneMesh& plane); //Get distance from point to plane
		uint32_t ReverseBits(uint32_t bits, uint32_t log2); //Reverse bits
        complex Noise(float limit); //Return noise
        complex Conjugate(complex c); //Conjugate complex number
}
