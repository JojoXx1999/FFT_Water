# FFT_Water
FFT Water Prototype
Created as my honours project at Abertay University this prototype uses an FFT model to simulate water
in real time, based on the work of Jerry Tessendorf.

Code is written in C++ and HLSL and uses DirectX11.

The majority of code to simulate the phyiscal moving on the water in 3D space and the ability to tile
the water object is found in E1_SimpleTerrain/Water.cpp

I created a maths file that is used heavily throughout the project, this can be found in E1_SimpleTerrain/Math.cpp

The HLSL code written for the water prototype creates both colour and lighting and can be found
in E1_SimpleTerrain/shaders/light_ps.hlsl

If running the game, use the arrow keys to change the camera rotation and W or S to move the camera
up and down respectively. 

Going to Properties - Philips Spectrum will give you the tools to change the physics of the water
simulation in real time. Color will allow you to change the colour and lighting.