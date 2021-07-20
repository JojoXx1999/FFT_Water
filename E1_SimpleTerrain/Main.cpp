// Main.cpp
#include "System.h"
#include "Main_App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow){
	MainApp* app = new MainApp();
	System* system;

	// Create the system object.
	system = new System(app, 1200, 675, true, false);

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}