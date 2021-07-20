#include "Main_App.h"

MainApp::MainApp()
{
	//Set initial variables
	m_light = nullptr;
	m_light_shader = nullptr;
	m_sun_light = nullptr;
	m_render_texture = nullptr;
	m_gui_handler = nullptr;

	for (int i = 0; i < sizeof(m_water) / sizeof(m_water[0]); i++)
	{
		m_water[i] = nullptr;
	}

	m_StartTime = std::chrono::system_clock::now();
	m_FFT_toggle = true;
}

void MainApp::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	//Load textures
	textureMgr->loadTexture(L"water", L"res/water.png");
	textureMgr->loadTexture(L"foam", L"res/foam.png");

	// Create Mesh object and shader object
	m_light_shader = new LightShader(renderer->getDevice(), hwnd);

	//Direct light colours
	m_specularColour = XMFLOAT3(1.0, 1.0f, 1.0);
	m_lightColour = XMFLOAT3(1.0, 0.89f, 0.8);

	//Sun light colours
	m_specularColour2 = XMFLOAT3(1.0, 1.0f, 1.0);
	m_lightColour2 = XMFLOAT3(1.0, 0.89f, 0.8);
	
	//Specular power
	m_specular_power1 = 0.2f;
	m_specular_power2 = 0.8f;

	//Water colours
	m_shallow_colour = XMFLOAT3(0.02, 0.23, 0.26);
	m_deep_colour = XMFLOAT3(0.0, 0.11, 0.16);
	m_atmosphere_colour = XMFLOAT3(0.12, 0.22, 0.24);

	//Camera variables
	//camera->setPosition(-39.7f, 87.f, -302.f);
	camera->setPosition(-9.75f, 50.f, -79.9f);
	camera->setRotation(10.0f, 0.f, 0.0f);

	//Light object
	m_light = new Light;
	m_light->setAmbientColour( 0.1f, 0.3f, 0.3f, 1.0f );
	m_light->setDiffuseColour(m_lightColour.x, m_lightColour.y, m_lightColour.z, 1.0f);
	m_light->setDirection(0.f,-0.8f, -0.9f);
	m_light->setSpecularColour(m_specularColour.x, m_specularColour.y, m_specularColour.z, 1.0);
	m_light->setSpecularPower(m_specular_power1);
	m_light->generateOrthoMatrix(SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 100.f);

	//Sunlight object
	m_sun_light = new Light;
	m_sun_light->setDiffuseColour(1.f, 0.0f, 0.f, 1.0f);
	m_sun_light->setDirection(0.f, -0.6f, -1.0f);
	m_sun_light->setPosition(-10.f, 57.0f, -100.f);
	m_sun_light->setSpecularColour(m_specularColour.x, m_specularColour.y, m_specularColour.z, 1.0);
	m_sun_light->setSpecularPower(m_specular_power2);
	m_sun_light->generateOrthoMatrix(SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 100.f);

	//Philips spectrum variables
	m_A = AMPLITUDE_CONSTANT;
	m_wind_speed = WIND_SPEED;

	//Load all water objects
	for (int i = 0; i < sizeof(m_water)/sizeof(m_water[0]); i++)
	{
		m_water[i] = new Water(renderer->getDevice(), renderer->getDeviceContext());
	}

	//For possible post-processing
	m_render_texture = new RenderTexture(renderer->getDevice(), SCREEN_WIDTH*2, SCREEN_HEIGHT*2, SCREEN_NEAR, SCREEN_FAR);

	//GUI
	m_gui_handler = new ImGuiHandler();
}


MainApp::~MainApp()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release objects
	SAFE_DELETE(m_light_shader);
	SAFE_DELETE(m_light);
	SAFE_DELETE(m_sun_light);
	SAFE_DELETE(m_render_texture);
	SAFE_DELETE(m_gui_handler);

	for (int i = 0; i < sizeof(m_water) / sizeof(m_water[0]); i++)
	{
		SAFE_DELETE(m_water[i]);
	}
}


bool MainApp::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

void MainApp::firstPass()
{
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	m_render_texture->setRenderTarget(renderer->getDeviceContext());
	m_render_texture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.f);

	//Get current time
	m_EndTime = std::chrono::system_clock::now();

	//Get time variable from current time for fft in nano seconds
	float time = (std::chrono::duration_cast<std::chrono::seconds>(m_EndTime - m_StartTime).count() + std::chrono::duration_cast<std::chrono::nanoseconds>(m_EndTime - m_StartTime).count()) / 1000000000.0;

	//Render water
	m_water[0]->render(time * m_gui_handler->getSpeed(), m_gui_handler->getLambda(), m_gui_handler->getT());
}

void MainApp::finalPass()
{
	renderer->resetViewport();
	renderer->setBackBufferRenderTarget();

	//Clear screen
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	//Update camera
	camera->update();

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	//Scale of water tiles
	m_water_scale = m_gui_handler->getTileSize();
	XMMATRIX scale = XMMatrixScaling(m_water_scale, m_water_scale, m_water_scale);

	//If water is tiled
	if (m_gui_handler->getTiled())
	{
		//For all tiles
		for (int j = 0; j < m_gui_handler->getVerticalGrid(); j++)
		{
			//Calculate Zoffset of tile
			int offsetZ = MESH_RESOLUTION * j;
			for (int i = 0; i < m_gui_handler->getHorizontalGrid(); i++)
			{
				//Calculate Xoffset of tile
				int offsetX = MESH_RESOLUTION * i;

				//Use position of vertices from original water tile for water tiles
				m_water[0]->useFFT(m_water[i+j], offsetX, offsetZ);

				//Generate water with offset
				m_water[i+j]->sendData(renderer->getDeviceContext());
				m_light_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, scale * viewMatrix, projectionMatrix, textureMgr->getTexture(L"foam"), textureMgr->getTexture(L"water"), m_light, m_sun_light,
					m_deep_colour, m_shallow_colour, m_atmosphere_colour, m_gui_handler->getReflectivity(), m_gui_handler->getReflect(), m_gui_handler->getReflect2(), m_gui_handler->getFoam(), 0, 0, camera->getPosition(), m_gui_handler->getMin(), m_gui_handler->getMax());
				m_light_shader->render(renderer->getDeviceContext(), m_water[i+j]->getIndexCount());

				//Generate water with negative offset
				offsetX = -offsetX;
				m_water[0]->useFFT(m_water[(m_gui_handler->getHorizontalGrid() + i) + (m_gui_handler->getVerticalGrid() + j)], offsetX, offsetZ);
				m_water[(m_gui_handler->getHorizontalGrid() + i) + (m_gui_handler->getVerticalGrid() + j)]->sendData(renderer->getDeviceContext());
				m_light_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, scale * viewMatrix, projectionMatrix, textureMgr->getTexture(L"foam"), textureMgr->getTexture(L"water"), m_light, m_sun_light,
					m_deep_colour, m_shallow_colour, m_atmosphere_colour, m_gui_handler->getReflectivity(), m_gui_handler->getReflect(), m_gui_handler->getReflect2(), m_gui_handler->getFoam(), 0, 0, camera->getPosition(), m_gui_handler->getMin(), m_gui_handler->getMax());
				m_light_shader->render(renderer->getDeviceContext(), m_water[(m_gui_handler->getHorizontalGrid() + i) + (m_gui_handler->getVerticalGrid() + j)]->getIndexCount());
			}
		}
	}
	else
	{
		//Generate single tile
		m_water[0]->sendData(renderer->getDeviceContext());
		m_light_shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, scale * viewMatrix, projectionMatrix, textureMgr->getTexture(L"foam"), textureMgr->getTexture(L"water"), m_light, m_sun_light,
			m_deep_colour, m_shallow_colour, m_atmosphere_colour, m_gui_handler->getReflectivity(), m_gui_handler->getReflect(), m_gui_handler->getReflect2(), m_gui_handler->getFoam(), 0.f, 0.f, camera->getPosition(), m_gui_handler->getMin(), m_gui_handler->getMax());
		m_light_shader->render(renderer->getDeviceContext(), m_water[0]->getIndexCount());
	}

	//Update variables
	UPDATE_VARIABLE(m_A, m_water[0]->Amplitude() * 10000);
	UPDATE_VARIABLE(m_wind_speed.x, m_water[0]->WindSpeed().x); UPDATE_VARIABLE(m_wind_speed.y, m_water[0]->WindSpeed().y);
	m_render_texture->clearRenderTarget(renderer->getDeviceContext(), 0.f, 0.f, 0.f, 1.f);
}

bool MainApp::render()
{
	firstPass();
	finalPass();	

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void MainApp::gui()
{
	//Render gui
	m_gui_handler->MenuBar(m_water[0], &m_A, &m_wind_speed, camera, &wireframeToggle, &m_deep_colour, &m_shallow_colour, &m_atmosphere_colour,
	&m_specular_power1, &m_specular_power2, &m_specularColour, &m_lightColour, &m_specularColour2, &m_lightColour2);
	

	//Update light variables
	if (m_light->getSpecularColour().x != m_specularColour.x || m_light->getSpecularColour().y != m_specularColour.y || m_light->getSpecularColour().z != m_specularColour.z)
		{
		m_light->setSpecularColour(m_specularColour.x, m_specularColour.y, m_specularColour.z, 1.0);
	}

	if (m_light->getDiffuseColour().x != m_lightColour.x || m_light->getDiffuseColour().y != m_lightColour.y || m_light->getDiffuseColour().z != m_lightColour.z)
	{
		m_light->setDiffuseColour(m_lightColour.x, m_lightColour.y, m_lightColour.z, 1.0f);
	}

	if (m_light->getSpecularPower() != m_specular_power1)
	{
		m_light->setSpecularPower(m_specular_power1);
	}

	if (m_sun_light->getSpecularColour().x != m_specularColour2.x || m_sun_light->getSpecularColour().y != m_specularColour2.y || m_sun_light->getSpecularColour().z != m_specularColour2.z)
	{
		m_sun_light->setSpecularColour(m_specularColour2.x, m_specularColour2.y, m_specularColour2.z, 1.0);
	}

	if (m_sun_light->getDiffuseColour().x != m_lightColour2.x || m_sun_light->getDiffuseColour().y != m_lightColour2.y || m_sun_light->getDiffuseColour().z != m_lightColour2.z)
	{
		m_sun_light->setDiffuseColour(m_lightColour2.x, m_lightColour2.y, m_lightColour2.z, 1.0f);
	}

	if (m_sun_light->getSpecularPower() != m_specular_power2)
	{
		m_sun_light->setSpecularPower(m_specular_power2);
	}
}

