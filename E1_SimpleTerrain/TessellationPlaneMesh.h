// Mesh for the height-mapped landscape in the scene. Set up to allow for tessellation.

#include "BaseMesh.h"

class TessellationPlaneMesh : public BaseMesh
{

public:

	TessellationPlaneMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 100);
	~TessellationPlaneMesh();

	void sendData(ID3D11DeviceContext* deviceContext);

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;
};