#include "MayaViewer.h"

// Declare our game instance
MayaViewer game;

MayaViewer::MayaViewer()
    : _scene(NULL), _wireframe(false)
{
}

bool MayaViewer::addMesh()
{

	// Calculate the vertices of the equilateral triangle.
	float a = 0.5f;     // length of the side
	Vector2 p1(0.0f, a / sqrtf(3.0f));
	Vector2 p2(-a / 2.0f, -a / (2.0f * sqrtf(3.0f)));
	Vector2 p3(a / 2.0f, -a / (2.0f * sqrtf(3.0f)));

	// Create 3 vertices. Each vertex has position (x, y, z) and color (red, green, blue)
	float vertices[] =
	{
		-0.5, 0.5, 0.0f,     1.0f, 0.0f, 0.0f,
		-0.5, -0.5, 0.0f,     0.0f, 1.0f, 0.0f,
		0.5, -0.5, 0.0f,     0.0f, 0.0f, 1.0f,
		// 2nd tri for quad
		0.5, -0.5, 0.0f,     0.0f, 0.0f, 1.0f,
		0.5, 0.5, 0.0f,     0.0f, 1.0f, 0.0f,
		-0.5, 0.5, 0.0f,     1.0f, 0.0f, 0.0f,

	};


	VertexFormat::Element elements[] =
	{
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3)
	};

	Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 2), 6, false);

	if (mesh == NULL)
	{
		GP_ERROR("Failed to create mesh.");
		return NULL;
	}

	mesh->setPrimitiveType(Mesh::TRIANGLES);
	mesh->setVertexData(vertices, 0, 6);

	Model* model = Model::create(mesh);
	Material* mat = model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "VERTEX_COLOR");

	SAFE_RELEASE(mesh);

	// These parameters are normally set in a .material file but this example sets them programmatically.
	// Bind the uniform "u_worldViewProjectionMatrix" to use the WORLD_VIEW_PROJECTION_MATRIX from the scene's active camera and the node that the model belongs to.
	mat->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mat->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	// Set the ambient color of the material.
	mat->getParameter("u_ambientColor")->setValue(Vector3(0.2f, 0.2f, 0.2f));



	// Create a white light.

	Vector3 col(0.75f, 0.75f, 0.75f);
	Light* light = Light::createPoint(col, 300.0f);
	Node* lightNode = _scene->addNode("light");
	lightNode->setLight(light);
	// Release the light because the node now holds a reference to it.
	SAFE_RELEASE(light);
	lightNode->translateUp(2.0f);
	// Bind the light's color and direction to the material.
	//mat->getParameter("u_directionalLightColor[0]")->setValue(lightNode->getLight()->getColor());
	//mat->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &Node::getForwardVectorWorld);

	_scene->addNode("Plane")->setDrawable(model);


	SAFE_RELEASE(model);

	// test change 1

	return true;
	
}

void MayaViewer::createMesh(char* data)
{
	Header h;
	size_t headerSize = sizeof(h);
	memcpy(&h, data, headerSize);

	MeshInfo newMesh;

	memcpy(&newMesh, data + headerSize, h.length);


	// Create 3 vertices. Each vertex has position (x, y, z) and color (red, green, blue)
	float vertices[] =
	{
		-0.5, 0.5, 0.0f,     1.0f, 0.0f, 0.0f,
		-0.5, -0.5, 0.0f,     0.0f, 1.0f, 0.0f,
		0.5, -0.5, 0.0f,     0.0f, 0.0f, 1.0f,
		// 2nd tri for quad
		0.5, -0.5, 0.0f,     0.0f, 0.0f, 1.0f,
		0.5, 0.5, 0.0f,     0.0f, 1.0f, 0.0f,
		-0.5, 0.5, 0.0f,     1.0f, 0.0f, 0.0f,

	};


	int vtxCount = newMesh.nrOfTriVertices;

	float* arr = new float[vtxCount * 2];

	float a = newMesh.vertices[newMesh.indices[0]].x;


	for (int i = 0; i < (vtxCount * 2); i++)
	{
		arr[i] = newMesh.vertices[newMesh.indices[i]].x;
		i++;
		arr[i] = newMesh.vertices[newMesh.indices[i]].y;
		i++;
		arr[i] = newMesh.vertices[newMesh.indices[i]].z;

		// debug only for temp colors
		i++;
		arr[i] = 1.0f;
		i++;
		arr[i] = 0.0f;
		i++;
		arr[i] = 0.0f;
		i++;

	}


	VertexFormat::Element elements[] =
	{
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3)
	};

	Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 2), vtxCount, false);

	if (mesh == NULL)
	{
		GP_ERROR("Failed to create mesh.");
		//return NULL;
	}

	mesh->setPrimitiveType(Mesh::TRIANGLES);
	mesh->setVertexData(arr, 0, vtxCount);

	Model* model = Model::create(mesh);
	Material* mat = model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "VERTEX_COLOR");

	SAFE_RELEASE(mesh);

	// These parameters are normally set in a .material file but this example sets them programmatically.
	// Bind the uniform "u_worldViewProjectionMatrix" to use the WORLD_VIEW_PROJECTION_MATRIX from the scene's active camera and the node that the model belongs to.
	mat->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mat->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	// Set the ambient color of the material.
	mat->getParameter("u_ambientColor")->setValue(Vector3(0.2f, 0.2f, 0.2f));


	// Create a white light.

	Vector3 col(0.75f, 0.75f, 0.75f);
	Light* light = Light::createPoint(col, 300.0f);
	Node* lightNode = _scene->addNode("light");
	lightNode->setLight(light);
	// Release the light because the node now holds a reference to it.
	SAFE_RELEASE(light);
	lightNode->translateUp(2.0f);
	// Bind the light's color and direction to the material.
	//mat->getParameter("u_directionalLightColor[0]")->setValue(lightNode->getLight()->getColor());
	//mat->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &Node::getForwardVectorWorld);

	_scene->addNode("newMesh")->setDrawable(model);


	SAFE_RELEASE(model);

}

void MayaViewer::initialize()
{
	// Create a new empty scene.
	_scene = Scene::create();

	// Create the camera.
	Camera* camera = Camera::createPerspective(45.0f, getAspectRatio(), 1.0f, 10.0f);
	Node* cameraNode = _scene->addNode("camera");

	// Attach the camera to a node. This determines the position of the camera.
	cameraNode->setCamera(camera);

	// Make this the active camera of the scene.
	_scene->setActiveCamera(camera);
	SAFE_RELEASE(camera);

	// Move the camera to look at the origin.
	cameraNode->translate(0, 1, 5);
	cameraNode->rotateX(MATH_DEG_TO_RAD(-11.25f));



	addMesh();

	


	// Visit all the nodes in the scene, drawing the models/mesh.
	//_scene->visit(this, &MayaViewer::initializeMaterials);
}

bool MayaViewer::initializeMaterials(Node* node)
{
	Model* model = dynamic_cast<Model*>(node->getDrawable());
	if (model)
	{
		Material* material = model->getMaterial();
		// For this sample we will only bind a single light to each object in the scene.
		MaterialParameter* colorParam = material->getParameter("u_directionalLightColor[0]");
		colorParam->setValue(Vector3(0.75f, 0.75f, 0.75f));
		MaterialParameter* directionParam = material->getParameter("u_directionalLightDirection[0]");
		directionParam->setValue(Vector3(0, -1, 0));
	}
	return true;
}

void MayaViewer::finalize()
{
    SAFE_RELEASE(_scene);
}

void MayaViewer::update(float elapsedTime)
{
 //   // Rotate model
 //   _scene->findNode("box")->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));

	//_scene->findNode("box")->translateUp(0.001);

	_scene->findNode("Plane")->translateUp(0.0001);

	_scene->findNode("Plane")->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));

	getMayaData();

	if (comlib.test() && onceBool)
	{
		_wireframe = !_wireframe;
		onceBool = false;
	}

}

void MayaViewer::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene for drawing
    _scene->visit(this, &MayaViewer::drawScene);
}

bool MayaViewer::drawScene(Node* node)
{
    // If the node visited contains a drawable object, draw it
    Drawable* drawable = node->getDrawable(); 
    if (drawable)
        drawable->draw(_wireframe);

    return true;
}


bool MayaViewer::test()
{
	return false;
}



void MayaViewer::getMayaData()
{

	char* data = new char[BUFF_SIZE];
	comlib.recieve(data);

	// data will be header + information
	// use msgtype in header to determine how to interpret information

	Header h;

	// Get header
	memcpy(&h, data, sizeof(h));

	switch (h.msgType)
	{
	case 1:
		createMesh(data);
		break;
	}


	// create new data type for specified information and pass it on to correct function
	// createMesh(dataType);
	// etc

	delete[] data;
}

void MayaViewer::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void MayaViewer::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        _wireframe = !_wireframe;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}
