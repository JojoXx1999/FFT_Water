#include "GUI_Handler.h"
#include "imGUI/imgui.h"
#include "imGUI/imgui_impl_dx11.h"
#include "imGUI/imgui_impl_win32.h"

ImGuiHandler::ImGuiHandler()
{
	//Set initial values
	m_diagnostics_toggle = true;
	m_physics_toggle = false;
	m_colour_toggle = false;

	//Light colour based on time of day
	m_dawn[0] = XMFLOAT3(0.99, 0.73, 0.37);
	m_dawn[1] = XMFLOAT3(0.99, 0.73f, 0.37f);
	m_high_Noon[0] = XMFLOAT3(1.0, 0.89f, 0.8);
	m_high_Noon[1] = XMFLOAT3(1.0, 1.f, 1.f);

	time_of_day = 0.6;
	reflectivityDirect = 0.4;
	reflectivitySpot = 0.5;
	light_Colour_Toggle = false;
	do_once = true;
	foam = 5.f;
	tiled = true;
	horizontalGrid = 3;
	verticalGrid = 3;
	tileSize = 3;
	//m_reflectivity = 1.4;
	m_reflectivity = 2.f;
	minHeight = 0.f;
	maxHeight = 1.66f;
	overideHeight = false;
	lambda = -1.8;
	T = 100.f;

	speed_multiplier = 1.f;
}

ImGuiHandler::~ImGuiHandler()
{

}

void ImGuiHandler::MenuBar(Water* water, float* A, XMFLOAT2* wind_Speed, Camera* camera, bool* wireframeToggle, XMFLOAT3* deep_color, XMFLOAT3* shallow_color, XMFLOAT3* atmosphere_color,
	float* specularPower1, float* specularPower2, XMFLOAT3* specularColour, XMFLOAT3* lightColour, XMFLOAT3* specularColour2, XMFLOAT3* lightColour2)
{
	m_water = water;
	m_A = *A;
	m_wind_Speed = *wind_Speed;
	m_camera = camera;
	m_wireframeToggle = wireframeToggle;

	m_shallow_Colour[0] = &shallow_color->x;
	m_shallow_Colour[1] = &shallow_color->y;
	m_shallow_Colour[2] = &shallow_color->z;

	m_deep_Colour[0] = &deep_color->x;
	m_deep_Colour[1] = &deep_color->y;
	m_deep_Colour[2] = &deep_color->z;

	m_atmosphere_Colour[0] = &atmosphere_color->x;
	m_atmosphere_Colour[1] = &atmosphere_color->y;
	m_atmosphere_Colour[2] = &atmosphere_color->z;

	m_specular_Power1 = specularPower1;
	m_specular_Power2 = specularPower2;

	m_specular_Colour1[0] = &specularColour->x;
	m_specular_Colour1[1] = &specularColour->y;
	m_specular_Colour1[2] = &specularColour->z;

	m_light_Colour1[0] = &lightColour->x;
	m_light_Colour1[1] = &lightColour->y;
	m_light_Colour1[2] = &lightColour->z;

	colour1[0] = lightColour->x;
	colour1[1] = lightColour->y;
	colour1[2] = lightColour->z;

	specColour1[0] = specularColour->x;
	specColour1[1] = specularColour->y;
	specColour1[2] = specularColour->z;

	m_specular_Colour2[0] = &specularColour2->x;
	m_specular_Colour2[1] = &specularColour2->y;
	m_specular_Colour2[2] = &specularColour2->z;

	m_light_Colour2[0] = &lightColour2->x;
	m_light_Colour2[1] = &lightColour2->y;
	m_light_Colour2[2] = &lightColour2->z;

	colour2[0] = lightColour2->x;
	colour2[1] = lightColour2->y;
	colour2[2] = lightColour2->z;

	specColour2[0] = specularColour2->x;
	specColour2[1] = specularColour2->y;
	specColour2[2] = specularColour2->z;

	if (do_once)
	{
		//Set initial light colours
		do_once = false;
		*m_light_Colour1[0] = m_dawn[0].x + (m_high_Noon[0].x - m_dawn[0].x) * time_of_day;
		*m_light_Colour1[1] = m_dawn[0].y + (m_high_Noon[0].y - m_dawn[0].y) * time_of_day;
		*m_light_Colour1[2] = m_dawn[0].z + (m_high_Noon[0].z - m_dawn[0].z) * time_of_day;

		*m_specular_Colour1[0] = m_dawn[1].x + (m_high_Noon[1].x - m_dawn[1].x) * time_of_day;
		*m_specular_Colour1[1] = m_dawn[1].y + (m_high_Noon[1].y - m_dawn[1].y) * time_of_day;
		*m_specular_Colour1[2] = m_dawn[1].z + (m_high_Noon[1].z - m_dawn[1].z) * time_of_day;

		*m_light_Colour2[0] = *m_light_Colour1[0];
		*m_light_Colour2[1] = *m_light_Colour1[1];
		*m_light_Colour2[2] = *m_light_Colour1[2];

		*m_specular_Colour2[0] = *m_specular_Colour1[0];
		*m_specular_Colour2[1] = *m_specular_Colour1[1];
		*m_specular_Colour2[2] = *m_specular_Colour1[2];
	}

	//If user wants to override the water colour
	if (overideHeight == false)
	{
		minHeight = 0.f * ((m_A / 10000) / (AMPLITUDE_CONSTANT));
		maxHeight = 2.8f * ((m_A / 10000) / (AMPLITUDE_CONSTANT));
	}


	//Set up GUI
	ImGui::StyleColorsClassic();
	ImGui::BeginMainMenuBar();

	File();

	Properties();

	Debug();

	ImGui::EndMainMenuBar();

	End();
}

void ImGuiHandler::File()
{
	//Close the application
	bool toggle = false;
	if (ImGui::BeginMenu("File"))
	{
		ImGui::MenuItem("Close Application", NULL, &toggle);

		ImGui::EndMenu();
	}

	if (toggle)
		exit(0);
}

void ImGuiHandler::Properties()
{
	//Change water in real time
	if (ImGui::BeginMenu("Properties"))
	{
		ImGui::MenuItem("Philips Spectrum", NULL, &m_physics_toggle);
		ImGui::MenuItem("Colour", NULL, &m_colour_toggle);

		ImGui::EndMenu();
	}

	if (m_physics_toggle)
		Physics();

	if (m_colour_toggle)
		Colour();
}

void ImGuiHandler::Colour()
{
	ImGui::Begin("Colour", &m_colour_toggle);

	//Variables affecting the visual aspect of the water
	ImGui::SliderFloat("Foam", &foam, 0.0, 5.f, "%.2f");
	ImGui::SliderFloat("Reflectivity", &m_reflectivity, 0, 3.0, "%.2f");
	if (ImGui::SliderFloat("min height", &minHeight, 0, 10, "%.2f")) overideHeight = true;
	if (ImGui::SliderFloat("max height", &maxHeight, 0, 10, "%.2f")) overideHeight = true;
	ImGui::SliderFloat("Specular Power", m_specular_Power1, 0, 1.0, "%.2f");
	ImGui::SliderFloat("Spot light specular Power", m_specular_Power2, 0, 1.0, "%.2f");

	if (ImGui::SliderFloat("Time of day", &time_of_day, 0.f, 1.f))
	{
		do_once = false;
		*m_light_Colour1[0] = m_dawn[0].x + (m_high_Noon[0].x - m_dawn[0].x) * time_of_day;
		*m_light_Colour1[1] = m_dawn[0].y + (m_high_Noon[0].y - m_dawn[0].y) * time_of_day;
		*m_light_Colour1[2] = m_dawn[0].z + (m_high_Noon[0].z - m_dawn[0].z) * time_of_day;

		*m_specular_Colour1[0] = m_dawn[1].x + (m_high_Noon[1].x - m_dawn[1].x) * time_of_day;
		*m_specular_Colour1[1] = m_dawn[1].y + (m_high_Noon[1].y - m_dawn[1].y) * time_of_day;
		*m_specular_Colour1[2] = m_dawn[1].z + (m_high_Noon[1].z - m_dawn[1].z) * time_of_day;

		*m_light_Colour2[0] = *m_light_Colour1[0];
		*m_light_Colour2[1] = *m_light_Colour1[1];
		*m_light_Colour2[2] = *m_light_Colour1[2];

		*m_specular_Colour2[0] = *m_specular_Colour1[0];
		*m_specular_Colour2[1] = *m_specular_Colour1[1];
		*m_specular_Colour2[2] = *m_specular_Colour1[2];
	}
	
	if (ImGui::ColorPicker3("Light Colour", &colour1[0]))
	{
		*m_light_Colour1[0] = colour1[0];
		*m_light_Colour1[1] = colour1[1];
		*m_light_Colour1[2] = colour1[2];
	}

	if (ImGui::ColorPicker3("Specular Colour", &specColour1[0]))
	{
		*m_specular_Colour1[0] = specColour1[0];
		*m_specular_Colour1[1] = specColour1[1];
		*m_specular_Colour1[2] = specColour1[2];
	}

	if (ImGui::ColorPicker3("Spot light Colour", &colour2[0]))
	{
		*m_light_Colour2[0] = colour2[0];
		*m_light_Colour2[1] = colour2[1];
		*m_light_Colour2[2] = colour2[2];
	}

	if (ImGui::ColorPicker3("Spot light specular Colour", &specColour2[0]))
	{
		*m_specular_Colour2[0] = specColour2[0];
		*m_specular_Colour2[1] = specColour2[1];
		*m_specular_Colour2[2] = specColour2[2];
	}

	ImGui::ColorPicker3("Shallow Water Colour", m_shallow_Colour[0]);
	ImGui::ColorPicker3("Deep Water Colour", m_deep_Colour[0]);
	ImGui::ColorPicker3("Atmosphere Colour", m_atmosphere_Colour[0]);

	ImGui::End();
}

void ImGuiHandler::Physics()
{
	//Variables affecting the physical water object(s)
	ImGui::Begin("Physics editor", &m_physics_toggle);

	ImGui::Text("All Changeable Variables Within for\nphysical attributes are listed below:");

	ImGui::SliderFloat("Speed Multiplier", &speed_multiplier, 0.0f, 5.f);
	ImGui::SliderFloat("Lambda", &lambda , -5.f, 5.f);
	ImGui::SliderFloat("T", &T, 0.f, 500.f);

	ImGui::Checkbox("Tiled", &tiled);
	ImGui::SliderInt("Horizontal Tiles", &horizontalGrid, 0, 5);
	ImGui::SliderInt("Vertical Tiles", &verticalGrid, 0, 5);
	ImGui::SliderFloat("Tile Size", &tileSize, 0.f, 5.f);

	if (ImGui::DragFloat("Wave Amplitude", &m_A, 0.1f, 0.f, 25.f, "%.2f"))
	{
		overideHeight = false;
		m_water->Amplitude(m_A / 10000);
		m_water->Generate();
	}

	if (ImGui::DragFloat2("Wind Speed", &m_wind_Speed.x, 1.f, 0.f, 1000.f, "%.2f"))
	{
		m_water->WindSpeed(m_wind_Speed);
		m_water->Generate();
	}

	ImGui::End();
}

void ImGuiHandler::Debug()
{
	//Display diagnostics window
	if (ImGui::BeginMenu("Debug"))
	{
		ImGui::MenuItem("Diagnostics", NULL, &m_diagnostics_toggle);

		ImGui::EndMenu();
	}

	if (m_diagnostics_toggle)
		Diagnostics();
}

void ImGuiHandler::Diagnostics()
{
	//Display debug information
	ImGui::Begin("Diagnostics", &m_diagnostics_toggle);

	ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", m_camera->getPosition().x, m_camera->getPosition().y, m_camera->getPosition().z);
	ImGui::Text("Camera Rot: (%.2f, %.2f, %.2f)", m_camera->getRotation().x, m_camera->getRotation().y, m_camera->getRotation().z);
	ImGui::Checkbox("Wireframe mode", m_wireframeToggle);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

void ImGuiHandler::End()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}