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
	mat->getParameter("u_directionalLightColor[0]")->setValue(lightNode->getLight()->getColor());
	mat->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &Node::getForwardVectorWorld);

	_scene->addNode("Plane")->setDrawable(model);

	SAFE_RELEASE(model);

	// test change 1

	return true;
	
}

void MayaViewer::createMesh(char* data)
{
	size_t headerSize   = sizeof(Header);
	size_t meshInfoSize = sizeof(MeshInfo);
	size_t localHead    = headerSize;

	MeshInfo newMesh;
	memcpy(&newMesh, data + localHead, meshInfoSize);
	localHead += meshInfoSize;


	size_t vtxSize      = sizeof(Vertex) * newMesh.nrOfVertices; // test
	size_t indiceSize   = sizeof(int) * newMesh.nrOfTriVertices;


	int vtxCount        = newMesh.nrOfVertices;
	int normalCount = newMesh.nrOfNormals;
	int indiceCount     = newMesh.nrOfTriVertices;

	float* vertices = new float[indiceCount * 8];


	// get vertices
	vector<Vertex> vtxVector;
	size_t vtxStructSize = sizeof(Vertex);

	Vertex v;
	for (int i = 0; i < vtxCount; i++)
	{
		// vector content (Vertex) has to be added back into a new vector
		memcpy(&v, data + localHead + (vtxStructSize * i), vtxStructSize);
		vtxVector.push_back(v);
	}
	
	localHead += vtxStructSize * vtxCount;

	// get indices
	vector<int> vtxIndicesVector;
	size_t sizeOfInt = sizeof(int);

	int vtxIndice;
	for (int i = 0; i < indiceCount; i++)
	{
		memcpy(&vtxIndice, data + localHead + (sizeOfInt * i), sizeOfInt);
		vtxIndicesVector.push_back(vtxIndice);
	}

	localHead += indiceCount * sizeOfInt;

	// get normals
	vector<Normal> normalVector;
	size_t sizeOfNormal = sizeof(Normal);

	Normal normal;
	for (int i = 0; i < normalCount; i++)
	{
		memcpy(&normal, data + localHead + (sizeOfNormal * i), sizeOfNormal);
		normalVector.push_back(normal);
	}

	localHead += normalCount * sizeOfNormal;


	// get normal indices
	vector<int> normalIndicesVector;
	int normIndice;
	for (int i = 0; i < indiceCount; i++)
	{
		memcpy(&normIndice, data + localHead + (sizeOfInt * i), sizeOfInt);
		normalIndicesVector.push_back(normIndice);
	}

	localHead += indiceCount * sizeOfInt;

	// get uvs
	vector<UV> uvVector;
	size_t sizeOfUv = sizeof(UV);

	UV uv;

	for (int i = 0; i < indiceCount; i++)
	{
		memcpy(&uv, data + localHead + (sizeOfUv * i), sizeOfUv);
		uvVector.push_back(uv);
	}

	localHead += indiceCount * sizeOfUv;


	// get uv indices
	vector<int> uvIndicesVector;

	int uvIndice;
	for (int i = 0; i < indiceCount; i++)
	{
		memcpy(&uvIndice, data + localHead + (sizeOfInt * i), sizeOfInt);
		uvIndicesVector.push_back(uvIndice);
	}




	int j = 0;
	for (int i = 0; i < indiceCount; i++)
	{
		// vertices
		vertices[j] = vtxVector[vtxIndicesVector[i]].x;
		j++;		
		vertices[j] = vtxVector[vtxIndicesVector[i]].y;
		j++;		
		vertices[j] = vtxVector[vtxIndicesVector[i]].z;
		j++;

		// normals
		vertices[j] = normalVector[normalIndicesVector[i]].x;
		j++;
		vertices[j] = normalVector[normalIndicesVector[i]].y;
		j++;
		vertices[j] = normalVector[normalIndicesVector[i]].z;
		j++;


		// uv
		vertices[j] = uvVector[uvIndicesVector[i]].x;
		j++;
		vertices[j] = uvVector[uvIndicesVector[i]].y;
		j++;
		

	}


	VertexFormat::Element elements[] =
	{
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3),
		VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
	};

	Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 3), indiceCount, false);

	if (mesh == NULL)
	{
		GP_ERROR("Failed to create mesh.");
		//return NULL;
	}

	mesh->setPrimitiveType(Mesh::TRIANGLES);
	mesh->setVertexData(vertices, 0, indiceCount);

	Model* model = Model::create(mesh);
	Material* mat = model->setMaterial("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");

	SAFE_RELEASE(mesh);

	// These parameters are normally set in a .material file but this example sets them programmatically.
	// Bind the uniform "u_worldViewProjectionMatrix" to use the WORLD_VIEW_PROJECTION_MATRIX from the scene's active camera and the node that the model belongs to.
	mat->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mat->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	// Set the ambient color of the material.
	mat->getParameter("u_ambientColor")->setValue(Vector3(0.3f, 0.3f, 0.3f));


	// Create a white light.

	Vector3 col(1.0f, 1.0f, 1.0f);
	Light* light = Light::createPoint(col, 16.0f);
	Node* lightNode = _scene->addNode("light");
	lightNode->setLight(light);
	// Release the light because the node now holds a reference to it.
	SAFE_RELEASE(light);
	lightNode->translateUp(1.0f);
	lightNode->translateLeft(3.0f);
	// Bind the light's color and direction to the material.
	mat->getParameter("u_pointLightColor[0]")->setValue(lightNode->getLight()->getColor());
	//mat->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &Node::getForwardVectorWorld);
	mat->getParameter("u_pointLightPosition[0]")->bindValue(lightNode, &Node::getTranslationWorld);
	mat->getParameter("u_pointLightRangeInverse[0]")->bindValue(lightNode->getLight(), &Light::getRangeInverse);
	mat->getParameter("u_diffuseColor")->setValue(Vector4(0.4f, 0.4f, 0.1f, 1.0f));

	//// Load the texture from file.
	Texture::Sampler* sampler = mat->getParameter("u_diffuseTexture")->setValue("res/png/crate.png", true);
	sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);
	mat->getStateBlock()->setCullFace(true);
	mat->getStateBlock()->setDepthTest(true);
	mat->getStateBlock()->setDepthWrite(true);
	


	meshList mlist;
	mlist.id = nrOfAddedMeshes;
	mlist.meshName = newMesh.name;

	string name = mlist.meshName;

	_scene->addNode(name.c_str())->setDrawable(model);


	string a = name.c_str();


	mVectorList.push_back(mlist);

	nrOfAddedMeshes++;

	SAFE_RELEASE(model);

	delete[] vertices;
}

void MayaViewer::transformMesh(char * data)
{

	size_t headerSize = sizeof(Header);
	size_t tdSize = sizeof(TranslationData);
	size_t localHead = headerSize;

	TranslationData td;
	memcpy(&td, data + localHead, tdSize);
	localHead += tdSize;

	Matrix m;

	Vector3 transVec;

	transVec.x = td.tx;
	transVec.y = td.ty;
	transVec.z = td.tz;

	m.translate(transVec);

	if (test2)
	{
		_scene->findNode(td.name)->setTranslation(transVec);
	}

}

void MayaViewer::scaleMesh(char * data)
{
	size_t headerSize = sizeof(Header);
	size_t sdSize = sizeof(ScaleData);
	size_t localHead = headerSize;

	ScaleData sd;
	memcpy(&sd, data + localHead, sdSize);
	localHead += sdSize;

	Matrix m;

	Vector3 scaleVec;

	scaleVec.x = sd.sx;
	scaleVec.y = sd.sy;
	scaleVec.z = sd.sz;

	//m.translate(scaleVec);

	if (test2)
	{
		_scene->findNode(sd.name)->setScale(scaleVec);
	}
}

void MayaViewer::rotateMesh(char * data)
{

	size_t headerSize = sizeof(Header);
	size_t rdSize = sizeof(RotationData);
	size_t localHead = headerSize;

	RotationData rd;
	memcpy(&rd, data + localHead, rdSize);
	localHead += rdSize;

	Matrix m;

	//m.translate(scaleVec);

	if (test2)
	{
		_scene->findNode(rd.name)->setRotation(rd.rx, rd.ry, rd.rz, rd.rw);
	}
}

void MayaViewer::updateCamera(char * data)
{
	size_t headerSize = sizeof(Header);
	size_t cdSize = sizeof(CameraData);
	size_t localHead = headerSize;

	CameraData cd;
	memcpy(&cd, data + localHead, cdSize);
	localHead += cdSize;

	Vector3 camTrans;
	camTrans.x = cd.posX;
	camTrans.y = cd.posY;
	camTrans.z = cd.posZ;


	/*auto setValues = [this](Vector3 camTrans, CameraData cd) {
		_scene->findNode("camera")->setTranslation(camTrans);
		_scene->findNode("camera")->setRotation(cd.lookAtX, cd.lookAtY, cd.lookAtZ, cd.lookAtW);
	};*/


	// Only update values
	//_scene->findNode("Ocamera")->setTranslation(camTrans);
	//_scene->findNode("Ocamera")->setRotation(cd.lookAtX, cd.lookAtY, cd.lookAtZ, cd.lookAtW);
	//_scene->findNode("Ocamera")->getCamera()->setZoomX(cd.zoom);
	//_scene->findNode("Ocamera")->getCamera()->setZoomY(cd.zoom);
	//_scene->setActiveCamera(_scene->findNode("Ocamera")->getCamera());


	

	if (cd.isOrtho) // If we want orthographic camera
	{
		
		if (isViewerCamOrtho) // Check if we already have it
		{
			// Only update values
			_scene->findNode("Ocamera")->setTranslation(camTrans);
			_scene->findNode("Ocamera")->setRotation(cd.lookAtX, cd.lookAtY, cd.lookAtZ, cd.lookAtW);
			_scene->findNode("Ocamera")->getCamera()->setZoomX(cd.zoom);
			_scene->findNode("Ocamera")->getCamera()->setZoomY(cd.zoom);
			_scene->findNode("Ocamera")->getCamera()->setAspectRatio(getAspectRatio());
			_scene->setActiveCamera(_scene->findNode("Ocamera")->getCamera());

			//setValues(camTrans, cd);
		}
		else { // If not
			if (!addedOCam)
			{

				// Create orthographic camera
				Camera* camera = Camera::createOrthographic(cd.zoom, cd.zoom, getAspectRatio(), cd.nearPlane, cd.farPlane);
				Node* cameraNode = _scene->addNode("Ocamera");

				// Attach the camera to a node. This determines the position of the camera.
				cameraNode->setCamera(camera);

				// Make this the active camera of the scene.
				_scene->setActiveCamera(camera);
				SAFE_RELEASE(camera);

				cameraNode->setTranslation(camTrans);
				cameraNode->setRotation(cd.lookAtX, cd.lookAtY, cd.lookAtZ, cd.lookAtW);
				cameraNode->getCamera()->setZoomX(cd.zoom);
				cameraNode->getCamera()->setZoomY(cd.zoom);

				//setValues(camTrans, cd);

				addedOCam = true;

			}
			isViewerCamOrtho = !isViewerCamOrtho;
		}
	}
	else { // If we want perspective camera
		if (isViewerCamOrtho) // Check if we need to switch back
		{
			if (!addedPCam)
			{

				// Create perspective camera
				Camera* camera = Camera::createPerspective(cd.fov, getAspectRatio(), cd.nearPlane, cd.farPlane);
				Node* cameraNode = _scene->addNode("Pcamera");

				// Attach the camera to a node. This determines the position of the camera.
				cameraNode->setCamera(camera);

				// Make this the active camera of the scene.
				_scene->setActiveCamera(camera);
				SAFE_RELEASE(camera);

				cameraNode->setTranslation(camTrans);
				cameraNode->setRotation(cd.lookAtX, cd.lookAtY, cd.lookAtZ, cd.lookAtW);

				//setValues(camTrans, cd);
				addedPCam = true;
			}

			isViewerCamOrtho = !isViewerCamOrtho;
		}
		else { // If not
			// Only update values
			_scene->findNode("Pcamera")->setTranslation(camTrans);
			_scene->findNode("Pcamera")->setRotation(cd.lookAtX, cd.lookAtY, cd.lookAtZ, cd.lookAtW);
			_scene->findNode("Pcamera")->getCamera()->setFieldOfView(cd.fov + 30);
			_scene->findNode("Pcamera")->getCamera()->setAspectRatio(getAspectRatio());
			_scene->setActiveCamera(_scene->findNode("Pcamera")->getCamera());
			//setValues(camTrans, cd);
		}
	}

	


}

void MayaViewer::updateMaterial(char * data)
{
	size_t headerSize = sizeof(Header);
	size_t mdSize = sizeof(MaterialData);
	size_t localHead = headerSize;

	MaterialData md;
	memcpy(&md, data + localHead, mdSize);
	localHead += mdSize;

	if (md.isTexture)
	{
		// Material in maya gets created before the node in gameplay, check if there is a node first
		Node* node = _scene->findNode(md.name);
		Model* model = nullptr;

		if (node)
		{
			model = dynamic_cast<Model*>(node->getDrawable());

			if (model)
			{
				replaceMaterialTexture(model, md);
			}

		}
	}
	else {

		// Material in maya gets created before the node in gameplay, check if there is a node first
		Node* node = _scene->findNode(md.name);
		Model* model = nullptr;

		if (node)
		{
			model = dynamic_cast<Model*>(node->getDrawable());

			if (model)
			{
				Vector4 color;
				color = md.rgb;
				color.w = 1.0f;

				replaceMaterialColor(model, color);


				/*Material * mat = model->getMaterial();

				MaterialParameter* colorParam = mat->getParameter("u_diffuseColor");
				colorParam->setValue(color);*/
			}
		}
	}


}

void MayaViewer::replaceMaterialColor(Model * model, Vector4 color)
{
	//Material * mat = model->getMaterial();

	Material * mat = model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");


	mat->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mat->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	mat->getParameter("u_ambientColor")->setValue(Vector3(0.3f, 0.3f, 0.3f));
	mat->getParameter("u_diffuseColor")->setValue(color);

	mat->getParameter("u_pointLightColor[0]")->setValue(_scene->findNode("light")->getLight()->getColor());
	//mat->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &Node::getForwardVectorWorld);
	mat->getParameter("u_pointLightPosition[0]")->bindValue(_scene->findNode("light"), &Node::getTranslationWorld);
	mat->getParameter("u_pointLightRangeInverse[0]")->bindValue(_scene->findNode("light")->getLight(), &Light::getRangeInverse);

	mat->getStateBlock()->setCullFace(true);
	mat->getStateBlock()->setDepthTest(true);
	mat->getStateBlock()->setDepthWrite(true);
}

void MayaViewer::replaceMaterialTexture(Model * model, MaterialData md)
{
	Material * mat = model->setMaterial("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");


	mat->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	mat->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	mat->getParameter("u_ambientColor")->setValue(Vector3(0.3f, 0.3f, 0.3f));
	//mat->getParameter("u_diffuseColor")->setValue(color);

	Texture::Sampler* sampler = mat->getParameter("u_diffuseTexture")->setValue(md.texturePath, true);
	sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);

	mat->getParameter("u_pointLightColor[0]")->setValue(_scene->findNode("light")->getLight()->getColor());
	//mat->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &Node::getForwardVectorWorld);
	mat->getParameter("u_pointLightPosition[0]")->bindValue(_scene->findNode("light"), &Node::getTranslationWorld);
	mat->getParameter("u_pointLightRangeInverse[0]")->bindValue(_scene->findNode("light")->getLight(), &Light::getRangeInverse);

	mat->getStateBlock()->setCullFace(true);
	mat->getStateBlock()->setDepthTest(true);
	mat->getStateBlock()->setDepthWrite(true);
}

void MayaViewer::nodeRemoved(char * data)
{
	size_t headerSize = sizeof(Header);
	size_t meshSize = sizeof(MeshInfo);
	size_t localHead = headerSize;

	MeshInfo mesh;
	memcpy(&mesh, data + localHead, meshSize);
	localHead += meshSize;

	_scene->removeNode(_scene->findNode(mesh.name));
}


void MayaViewer::initialize()
{
	// Create a new empty scene.
	_scene = Scene::create();

	// Create the camera.
	Camera* camera = Camera::createPerspective(75.0f, getAspectRatio(), 1.0f, 10000.0f);
	Node* cameraNode = _scene->addNode("camera");

	Camera* cameraP = Camera::createPerspective(75.0f, getAspectRatio(), 1.0f, 100.0f);
	Node* cameraNodeP = _scene->addNode("Pcamera");
	cameraNodeP->setCamera(cameraP);


	/*
	Camera* cameraO = Camera::createOrthographic(10.0f, 10.0f, getAspectRatio(), 1.0f, 10000.0f);
	Node* cameraNodeO = _scene->addNode("Ocamera");
	cameraNodeO->setCamera(cameraO);
	*/

	// Attach the camera to a node. This determines the position of the camera.
	cameraNode->setCamera(camera);


	// Make this the active camera of the scene.
	_scene->setActiveCamera(cameraP);
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

	//if (test2)
	//{

	//	for (int i = 0; i < mVectorList.size(); i++)
	//	{
	//		_scene->findNode(mVectorList[i].meshName.c_str())->translateLeft(-0.01);
	//		_scene->findNode(mVectorList[i].meshName.c_str())->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	//		_scene->findNode(mVectorList[i].meshName.c_str())->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	//	}
	//	//test2 = false;
	//}

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
	case MSG_TYPE::MeshAdded:
		createMesh(data);
		test2 = true;
		break;

	case MSG_TYPE::Translation:
		transformMesh(data);
		break;
	case MSG_TYPE::Rotation:
		rotateMesh(data);
		break;
	case MSG_TYPE::Scale:
		scaleMesh(data);
		break;
	case MSG_TYPE::Camera_Update:
		updateCamera(data);
		break;
	case MSG_TYPE::Material_Change:
		updateMaterial(data);
		break;
	case MSG_TYPE::Node_Removed:
		nodeRemoved(data);
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
