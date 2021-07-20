#pragma once
//Global macros to ease the programming process
class Vertex;

//Get the minimum and maximum between values
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))

//Clean up variables
#define SAFE_RELEASE(v) { if ( (v) ) { (v)->Release(); (v) = NULL; } }
#define SAFE_DELETE(v) if( (v) != NULL ) delete (v); (v) = NULL;
#define SAFE_DELETE_ARRAY(v) if( (v) != NULL ) delete[] (v); (v) = NULL;

//Update variable
#define UPDATE_VARIABLE(v, n) if (v != n) v = n;

//Create new variable of object type
#define NEW_COMPLEX(v, s) if (v == NULL) v = new Math::complex[s * s];
#define NEW_FFT(fft, s) if (fft == NULL) fft = new FFT(s);
#define NEW_VERTEX_WATER(water, s) if (water == NULL) water = new Vertex[s * s];

//Constant values
#define PI 3.14159265358979323846
#define MESH_RESOLUTION 128
#define GRAVITY 9.81f
#define WIND_SPEED {9.0f, 5.0f}
#define AMPLITUDE_CONSTANT 0.00004f
#define PI2 (PI*2)
#define EPSILON 0.000001

#define SCREEN_WIDTH 700.f
#define SCREEN_HEIGHT 700.f
#define SCREEN_NEAR 0.1f
#define SCREEN_FAR 1000.f



