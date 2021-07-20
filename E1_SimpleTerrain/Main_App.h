#pragma once

// Includes
#include "DXF.h"	// include dxframework
#include "Light_Shader.h"
#include <time.h>
#include <chrono>
#include "GUI_Handler.h"

class MainApp : public BaseApplication
{
public:
	MainApp();
	~MainApp();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);
	bool frame();
	
protected:
	bool render();
	void gui();

private:
	//Set up for possible post processing
	void firstPass();
	void finalPass();

	//Use FFT or DFT
	bool m_FFT_toggle;

	//Light variables
	LightShader* m_light_shader;
	Light* m_light, *m_sun_light;

	//Render texture for post processing
	RenderTexture* m_render_texture;

	//Include GUI
	ImGuiHandler* m_gui_handler;

	//Water objects
	Water* m_water[25];

	//Variables for philips spectrum
	float m_A;
	XMFLOAT2 m_wind_speed;

	//Colour variables
	XMFLOAT3 m_deep_colour, m_shallow_colour, m_atmosphere_colour;

	//Light variables
	XMFLOAT3 m_specularColour, m_lightColour, m_specularColour2, m_lightColour2;
	float m_specular_power1, m_specular_power2, m_specular_reflectance, m_water_scale;

	//Time used for fft simulation
	std::chrono::time_point<std::chrono::system_clock> m_StartTime;
	std::chrono::time_point<std::chrono::system_clock> m_EndTime;
};
