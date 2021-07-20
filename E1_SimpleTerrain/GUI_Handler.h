#pragma once

#include "Water.h"
#include "FPCamera.h"

class ImGuiHandler
{
public:
	ImGuiHandler();
	~ImGuiHandler();
	void MenuBar(Water* water, float* A, XMFLOAT2* wind_Speed, Camera* camera, bool* wireframeToggle, XMFLOAT3* deep_color, XMFLOAT3* shallow_color, XMFLOAT3* atmosphere_color,
	float* specularPower1, float* specularPower2, XMFLOAT3* specularColour, XMFLOAT3* lightColour, XMFLOAT3* specularColour2, XMFLOAT3* lightColour2);

	//Getters
	inline float getReflect() { return reflectivityDirect; }
	inline float getReflect2() { return reflectivitySpot; }
	inline float getFoam() { return foam; }
	inline bool getTiled() { return tiled; }
	inline int getVerticalGrid() { return verticalGrid; }
	inline int getHorizontalGrid() { return horizontalGrid; }
	inline float getTileSize() { return tileSize; }
	inline float getMin() { return minHeight; }
	inline float getMax() { return maxHeight; }
	inline float getReflectivity() { return m_reflectivity; }
	inline float getLambda() { return lambda; }
	inline float getSpeed() { return speed_multiplier; }
	inline float getT() { return T; }

private:
	//GUI options
	void File();
	void Debug();
	void Diagnostics();
	void Properties();
	void Physics();
	void Colour();
	void End();

	//Get amplitude or wind speed
	inline float* getA() { return &m_A; }
	inline XMFLOAT2* getWindSpeed() { return &m_wind_Speed; }

	//Toggles for rendering gui windows
	bool m_diagnostics_toggle, m_physics_toggle, m_colour_toggle;
	float m_A;
	XMFLOAT2 m_wind_Speed;
	Water* m_water;
	Camera* m_camera;
	bool* m_wireframeToggle;

	//Water Colour
	float* m_deep_Colour[3];
	float* m_shallow_Colour[3];
	float* m_atmosphere_Colour[3];

	//Variables for light
	float m_reflectivity, *m_specular_Power1, * m_specular_Power2;
	float* m_specular_Colour1[3], * m_specular_Colour2[3], *m_light_Colour1[3], *m_light_Colour2[3];
	float reflectivityDirect, reflectivitySpot, time_of_day;
	bool light_Colour_Toggle;

	//Variables for water colour
	float colour1[3], specColour1[3], colour2[3], specColour2[3], foam, minHeight, maxHeight;
	XMFLOAT3 m_high_Noon[2], m_dawn[2];
	bool overideHeight;

	//Tile water
	bool tiled;
	float tileSize;
	int verticalGrid, horizontalGrid;

	//debounce
	bool do_once;
	
	//variables for fft/dft
	float lambda, speed_multiplier;

	//Time for repeating dispersion
	float T;
};
