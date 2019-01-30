// UD1414_Plugin.cpp : Defines the exported functions for the DLL application.

#include "Maya_Includes.h"
#include <iostream>
#include <string>
#include "ComLib.h"
#include "Data.h"
#include <vector>
#include <ctime>

using namespace std;
MCallbackIdArray myCallbackArray;

// Declarations start ***************************************
void nodeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData);

void meshAdded(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData);

void prepareMeshMessage(MeshInfo mesh, vectorData vD, MSG_TYPE msgType);

int calculateLocalIndex(MPointArray localPointArray, MFloatPointArray objPoints, int vertexListIndex);

void prepareTranslationMessage(TranslationData td);

void prepareScaleMessage(ScaleData sd);

void prepareRotationMessage(RotationData rd);

void prepareCameraMessage(CameraData cam);

void prepareMaterialMessage(MaterialData md);

void prepareRemovedMessage(MeshInfo mesh);

// Declarations end *****************************************

MCallbackId meshNodeCallbackID;

ComLib* comLib;

float fps = 30;

clock_t elapsedTime;
bool g_TransDataUpdated = false;
bool g_ScaleDataUpdated = false;
bool g_RotDataUpdated = false;

TranslationData g_TransData;
ScaleData g_ScaleData;
RotationData g_RotData;

CameraData g_CamData;
bool g_CamDataUpdated = false;

MString getNodeName(MObject &node) {
	MString nodeName;

	if (node.hasFn(MFn::kDagNode))
	{
		MFnDagNode dagObj(node);
		nodeName = dagObj.fullPathName();
	}
	else {
		MFnDependencyNode dn(node);
		nodeName = dn.name();
	}

	return nodeName;
}
void TimerCallBack(float elapsedTime, float lastTime, void *clientData) {
	/*MString msg("Elapsd time: ");
	msg += elapsedTime;
	MGlobal::displayInfo(msg);*/

	//time += elapsedTime;

	if (g_TransDataUpdated)
	{
		MGlobal::displayInfo(MString("Sending translation data"));
		prepareTranslationMessage(g_TransData);
		g_TransDataUpdated = false;
	}

	if (g_ScaleDataUpdated)
	{
		MGlobal::displayInfo(MString("Sending scale data"));
		prepareScaleMessage(g_ScaleData);
		g_ScaleDataUpdated = false;
	}

	if (g_RotDataUpdated)
	{
		MGlobal::displayInfo(MString("Sending rotation data"));
		prepareRotationMessage(g_RotData);
		g_RotDataUpdated = false;
	}

	if (g_CamDataUpdated)
	{
		prepareCameraMessage(g_CamData);
		g_CamDataUpdated = false;
	}
}

void cameraViewChanged(const MString &str, void *clientData) {

	//MGlobal::displayInfo(MString("Camera changing!!"));

	M3dView view;

	MStatus status;

	status = M3dView::getM3dViewFromModelPanel(str, view);

	if (status == MS::kSuccess)
	{
		MDagPath camPath;

		view.getCamera(camPath);

		MFnCamera cam(camPath);

		MPoint camPos;

		camPos = cam.eyePoint(MSpace::kWorld);

		MFnTransform camLookAt(cam.parent(0));

		double rotX, rotY, rotZ, rotW;
		camLookAt.getRotationQuaternion(rotX, rotY, rotZ, rotW);

		g_CamData.posX = camPos.x;
		g_CamData.posY = camPos.y;
		g_CamData.posZ = camPos.z;

		g_CamData.lookAtX = rotX;
		g_CamData.lookAtY = rotY;
		g_CamData.lookAtZ = rotZ;
		g_CamData.lookAtW = rotW;

		g_CamData.nearPlane = cam.nearClippingPlane();
		g_CamData.farPlane = cam.farClippingPlane();

		g_CamData.fov = cam.horizontalFieldOfView();

		g_CamData.zoom = cam.orthoWidth();

		g_CamData.aspectRatio = cam.aspectRatio();

		if (cam.isOrtho())
		{
			g_CamData.isOrtho = true;
		}
		else {
			g_CamData.isOrtho = false;
		}

		MString a;

		a += camPos.x;
		a += " ,";
		a += camPos.y;
		a += " ,";
		a += camPos.z;
		a += " ,";
		a += cam.aspectRatio();
		a += " ,";


		MGlobal::displayInfo(a);

		clock_t timer;

		timer = clock() - elapsedTime;

		if (timer > 1000 / fps) 
		{
			prepareCameraMessage(g_CamData);

			elapsedTime = clock();
		}



		g_CamDataUpdated = true;
	}


}

void prepareCameraMessage(CameraData cam) {
	Header h;

	size_t headerSize = sizeof(Header);
	size_t camSize = sizeof(cam);

	size_t localHead = 0;

	h.msgType = MSG_TYPE::Camera_Update;
	h.length = camSize;


	char* data = new char[headerSize + h.length];


	// Header
	memcpy(data, &h, headerSize);
	localHead += headerSize;

	// cam
	memcpy(data + localHead, &cam, camSize);


	comLib->send(data, headerSize + h.length);

	delete[] data;

}

void TransformChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData) {

	MString tmsg("Transform changed!");
	MGlobal::displayInfo(tmsg);
}

void NodeAdded(MObject &node, void *clientData) {
	MString nodeName = getNodeName(node);

	// node parent is always transform node
	// callback in here - attribute changed
	// add the transform node into a node changed callback


	// Mesh node changed
	MCallbackId nodeAttrChange;
	MStatus nodeAttrChangedStatus;
	if (node.apiType() == MFn::kMesh)
	{
		nodeAttrChange = MNodeMessage::addAttributeChangedCallback(node, nodeChanged, NULL, &nodeAttrChangedStatus);
		if (nodeAttrChangedStatus == MS::kSuccess)
		{
			if (myCallbackArray.append(nodeAttrChange) == MS::kSuccess)
			{
				MGlobal::displayInfo(MString("NodeChanged successfully added to callback array"));
			}
		}


	}
	else if (node.apiType() == MFn::kTransform)
	{
		MGlobal::displayInfo(MString("----TRANSFORM NODE Found     !!"));
		MCallbackId transformNodeAttrChange;
		MStatus transformNodeAttrChangedStatus;

		transformNodeAttrChange = MNodeMessage::addAttributeChangedCallback(node, nodeChanged, NULL, &transformNodeAttrChangedStatus);
		if (transformNodeAttrChangedStatus == MS::kSuccess)
		{
			if (myCallbackArray.append(transformNodeAttrChange) == MS::kSuccess)
			{
				MGlobal::displayInfo(MString("Transform node added to callback array"));
			}
		}
	}


	MString msg("NODE ADDED! ->  " + nodeName);
	MGlobal::displayInfo(msg);



	if (node.hasFn(MFn::Type::kMesh))
	{
		MString msg(nodeName + " Has been added  ---- !! ---- New");
		MGlobal::displayInfo(msg);

		// check dag node then
		// add new function to callback array to check if node is changed. Dag node is added once per mesh, 
		// so the callback will also only be added once per mesh. Hopefully

		MStatus dagStatus;
		MFnDagNode dagNode(node, &dagStatus);

		if (dagStatus == MS::kSuccess)
		{
			MString msg("Dag node has been found");
			MGlobal::displayInfo(msg);

			// Add callback
			MStatus callbackStatus;
			meshNodeCallbackID = MNodeMessage::addAttributeChangedCallback(node, meshAdded, NULL, &callbackStatus);

			if (callbackStatus == MS::kSuccess)
			{
				msg = "meshAdded callback status = success";
				MGlobal::displayInfo(msg);
				// Add to callback array
				myCallbackArray.append(meshNodeCallbackID);
			}

		}

	}


	// Comlib test

	//comLib->test();
}

void meshAdded(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData) {

	MGlobal::displayInfo(MString("--------- meshAdded function:"));
	MGlobal::displayInfo(MString("Plug api type: ") + plug.node().apiTypeStr() + " " + msg);

	MStatus status;
	MFnMesh fn(plug.node(), &status);

	
	if (status == MS::kSuccess)
	{
		MGlobal::displayInfo(MString("Status  ==  Succsess ") + getNodeName(plug.node()));

		// Local data for mesh
		MeshInfo   mesh;
		vectorData vD;


		// Maya classes
		MFloatPointArray  meshVertexPoints;
		MFloatVectorArray normalVector;
		MFloatArray       uvArrX;
		MFloatArray       uvArrY;
		MIntArray         triangleCounts;
		MIntArray         triangleVertices;		// Holds total amount of vertices (indices array), not used in viewer!
		MIntArray         normalCounts;			// Debug
		MIntArray         normals;				// Debug
		// Iterator variables
		MPointArray       localPoints;
		MPointArray       tempPoints;
		MIntArray         vertexList;


		// Custom temporary structs for mesh
		UV     uv;
		Normal normal;
		Vertex v;
		int    vertexIndice;
		int    normalIndice;
		int    uvIndex;


		// Maya functions to access data
		fn.getPoints   (meshVertexPoints);
		fn.getUVs      (uvArrX, uvArrY);
		fn.getNormals  (normalVector);
		fn.getTriangles(triangleCounts, triangleVertices);
		fn.getNormalIds(normalCounts, normals); // Debug


		// Add mesh data to MeshInfo struct
		mesh.nrOfVertices    = meshVertexPoints.length();
		mesh.nrOfTriVertices = triangleVertices.length();
		mesh.nrOfNormals     = normalVector.length();
		mesh.msgType         = 1;
		strncpy(mesh.name, getNodeName(fn.parent(0)).asChar(), sizeof(mesh.name));
	


		// Push UVs into vector
		for (int i = 0; i < fn.numUVs(); i++)
		{
			uv.x = uvArrX[i];
			uv.y = uvArrY[i];

			vD.uv.push_back(uv);
		}

		// Push normals into vector 
		for (int i = 0; i < normalVector.length(); i++)
		{
			normal.x = normalVector[i].x;
			normal.y = normalVector[i].y;
			normal.z = normalVector[i].z;

			vD.normal.push_back(normal);
		}

		// Push vertices into vector
		for (unsigned int i = 0; i < meshVertexPoints.length(); i++)
		{
			v.x = meshVertexPoints[i].x;
			v.y = meshVertexPoints[i].y;
			v.z = meshVertexPoints[i].z;

			vD.v.push_back(v);
		}

		MStatus ItTestStatus;

		MItMeshPolygon meshIteratorObj(plug.node(), &ItTestStatus);
		
		// iterate through every face (polygon) of the mesh
		for (; !meshIteratorObj.isDone(); meshIteratorObj.next())
		{
			// 0 - 23 normalIndex(int localVertexIndex ) - Returns the normal index for the specified vertex.
			// point(int index) - Return the position of the vertex at index in the current polygon.
			// getPoints(MPointArray & 	pointArray) - Retrieves the positions of the vertices on the current face/polygon that the iterator is pointing to.
			// getTriangles(MPointArray & points, MIntArray & vertexList) - Get the vertices and vertex positions of all the triangles in the current face's triangulation.
			// 0 - 7 vertexIndex(int index) - Returns the object-relative index of the specified vertex of the current polygon. !! <---


			meshIteratorObj.getPoints(localPoints);
			meshIteratorObj.getTriangles(tempPoints, vertexList);

#pragma region Debug
			MString str;
			str = "";

			for (int i = 0; i < vertexList.length(); i++)
			{
				str += vertexList[i];
				str += ", ";
			}

			str += "\n------ \n";
			MGlobal::displayInfo(str);

#pragma endregion
			// Push vertex indices into vector
			for (int i = 0; i < vertexList.length(); i++)
			{
				vertexIndice = vertexList[i];
				vD.vertexIndices.push_back(vertexIndice);
			}

			// Push normal and uv indices into vector
			for (int i = 0; i < vertexList.length(); i++)
			{
				int localIndex = calculateLocalIndex(localPoints, meshVertexPoints, vertexList[i]);
#pragma region Debug
				str = "LocalIndex is now: ";
				str += localIndex;
				MGlobal::displayInfo(str);

				str = "NormalVectorIndex is now: ";
				str += meshIteratorObj.normalIndex(localIndex);
				MGlobal::displayInfo(str);
#pragma endregion
				// Normals
				normalIndice = meshIteratorObj.normalIndex(localIndex);
				vD.normalIndices.push_back(normalIndice);

				// UVs
				uvIndex;
				meshIteratorObj.getUVIndex(localIndex, uvIndex);

				vD.uvIndices.push_back(uvIndex);
			}


		}



#pragma region Debug
		// Debug
		MString points;
		for (unsigned int i = 0; i < meshVertexPoints.length(); i++)
		{
			points += "Vertex ";
			points += i;
			points += "::: x: ";
			points += meshVertexPoints[i].x;
			points += ", y: ";
			points += meshVertexPoints[i].y;
			points += ", z: ";
			points += meshVertexPoints[i].z;
			points += "\n";
		}

		MGlobal::displayInfo(points);

		// triangleCounts
		MString msg(getNodeName(plug.node()) + ": Triangle counts array length  " + triangleCounts.length());
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": Triangle counts array ");
		for (unsigned int i = 0; i < triangleCounts.length(); i++)
		{
			msg += triangleCounts[i];
			msg += ", ";
		}
		MGlobal::displayInfo(msg);

		// triangleVertices important
		msg = (getNodeName(plug.node()) + ": Triangle vertices array length  " + triangleVertices.length());
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": Triangle vertices array ");
		for (unsigned int i = 0; i < triangleVertices.length(); i++)
		{
			msg += triangleVertices[i];
			msg += ", ";
		}

		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": UV array X:  " + fn.numUVs() + " total ");
		for (int i = 0; i < fn.numUVs(); i++)
		{
			msg += uvArrX[i];
			msg += ", ";
		}

		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": UV array Y:  " + fn.numUVs() + " total ");
		for (int i = 0; i < fn.numUVs(); i++)
		{
			msg += uvArrY[i];
			msg += ", ";
		}
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": Normals: " + " Length = " + normalVector.length() + "\n");
		for (unsigned int i = 0; i < normalVector.length(); i++)
		{
			msg += "Normal ";
			msg += i;
			msg += ": x: ";
			msg += normalVector[i].x;
			msg += " y: ";
			msg += normalVector[i].y;
			msg += " z: ";
			msg += normalVector[i].z;
			msg += "\n";
		}
		MGlobal::displayInfo(msg);


		msg = (getNodeName(plug.node()) + ": Normal indice array: ");
		for (int i = 0; i < mesh.nrOfTriVertices; i++)
		{
			msg += vD.normalIndices[i];
			msg += ", ";
		}
		MGlobal::displayInfo(msg);


		msg = (getNodeName(plug.node()) + ": normal counts ");
		for (int i = 0; i < normalCounts.length(); i++)
		{
			msg += normalCounts[i];
			msg += ", ";
		}
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": normal indices ");
		for (int i = 0; i < normals.length(); i++)
		{
			msg += normals[i];
			msg += ", ";
		}
		MGlobal::displayInfo(msg);
		

#pragma endregion

		// remove callback(s)
		MNodeMessage::removeCallback(meshNodeCallbackID);

		//myCallbackArray.remove(meshNodeCallbackID);
		MGlobal::displayInfo(MString("Callback removed"));


		prepareMeshMessage(mesh, vD, MSG_TYPE::MeshAdded);
		
	}
}

int calculateLocalIndex(MPointArray localPointArray, MFloatPointArray objPoints, int vertexListIndex) {

	int index;
		for (int i = 0; i < localPointArray.length(); i++)
		{
			if (localPointArray[i] == objPoints[vertexListIndex])
			{
				index = i;
			}
		}
	
	return index;
}

void prepareMeshMessage(MeshInfo mesh, vectorData vD, MSG_TYPE msgType) {

	// create local mesh info holding all vector stuff
	// dont copy over struct containing vector


	Header h;

	int totalVertices       = mesh.nrOfTriVertices;
	size_t headerSize       = sizeof(h);
	size_t meshSize         = sizeof(mesh);
	size_t vtxSize          = sizeof(Vertex) * mesh.nrOfVertices; // test
	size_t vtxIndiceSize    = sizeof(int) * totalVertices;
	size_t normalSize       = sizeof(Normal) * mesh.nrOfNormals;
	size_t normalIndiceSize = sizeof(int) * totalVertices;
	size_t uvSize           = sizeof(UV) * totalVertices;
	size_t uvIndiceSize     = sizeof(int) * totalVertices;
	size_t localHead        = 0;

	h.msgType = msgType;
	h.length = meshSize + vtxSize + vtxIndiceSize + normalSize + normalIndiceSize + uvSize + uvIndiceSize;



	char* data = new char[headerSize + h.length];

	// Header
	memcpy(data, &h, headerSize);
	localHead   += headerSize;

	// Meshinfo
	memcpy(data + localHead, &mesh, meshSize);
	localHead   += meshSize;

	// Vertices
	memcpy(data + localHead, vD.v.data(), vtxSize);
	localHead   += vtxSize;

	// Vertex Indices
	memcpy(data + localHead, vD.vertexIndices.data(), vtxIndiceSize);
	localHead   += vtxIndiceSize;

	// Normals
	memcpy(data + localHead, vD.normal.data(), normalSize);
	localHead += normalSize;

	// Normal indices
	memcpy(data + localHead, vD.normalIndices.data(), normalIndiceSize);
	localHead += normalIndiceSize;

	// UVs
	memcpy(data + localHead, vD.uv.data(), uvSize);
	localHead += uvSize;

	// UV indices
	memcpy(data + localHead, vD.uvIndices.data(), uvIndiceSize);
	localHead += uvIndiceSize;



	comLib->send(data, headerSize + h.length);


	delete[] data;
}

void nodeRenamed(MObject &node, const MString &str, void *clientData) {
	MString nodeName = getNodeName(node);
	MString msg(str + " has been renamed to " + nodeName);
	MGlobal::displayInfo(msg);
}

void nodeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData) {

	//MGlobal::displayInfo(MString("Node changed function:"));

	MStatus status;
	MFnMesh fn(plug.node(), &status);

	// try to get translation, scale and rot data here



	if (plug.partialName(0, 0, 0, 0, 0, 1) == "translateX" || plug.partialName(0, 0, 0, 0, 0, 1) == "translateY" || plug.partialName(0, 0, 0, 0, 0, 1) == "translateZ")
	{
		MFnTransform transFn(plug.node(), &status);

		MStatus test;
		MTransformationMatrix tMtx = transFn.transformation(&test);

		MString n = transFn.name();


		MString msg;
		if (test == MS::kSuccess)
		{
			//MGlobal::displayInfo(MString("TRANSFORM MATRIX -----"));
			
			msg = "Translation data: ";
			msg += tMtx.getTranslation(MSpace::kWorld).x;
			msg += ", ";
			msg += tMtx.getTranslation(MSpace::kWorld).y;
			msg += ", ";
			msg += tMtx.getTranslation(MSpace::kWorld).z;
			MGlobal::displayInfo(msg);


			TranslationData td;

			td.tx = (float)tMtx.getTranslation(MSpace::kWorld).x;

			td.ty = (float)tMtx.getTranslation(MSpace::kWorld).y;

			td.tz = (float)tMtx.getTranslation(MSpace::kWorld).z;

			strncpy(td.name, getNodeName(plug.node()).asChar(), sizeof(td.name));

			g_TransData = td;
			g_TransDataUpdated = true;

		}

	}
	if (plug.partialName(0, 0, 0, 0, 0, 1) == "rotateX" || plug.partialName(0, 0, 0, 0, 0, 1) == "rotateY" || plug.partialName(0, 0, 0, 0, 0, 1) == "rotateZ")
	{
		MFnTransform transFn(plug.node(), &status);


		double rotX, rotY, rotZ, rotW;

		MStatus test;
		MTransformationMatrix tMtx = transFn.transformation(&test);


		MString msg;
		if (test == MS::kSuccess)
		{

			tMtx.getRotationQuaternion(rotX, rotY, rotZ, rotW);

			msg = "Rotation data: \n";
			msg += rotX;
			msg += ", ";
			msg += rotY;
			msg += ", ";
			msg += rotZ;
			msg += ", ";
			msg += rotW;
			msg += ", ";

			MGlobal::displayInfo(msg);

			RotationData rd;

			rd.rx = rotX;
			rd.ry = rotY;
			rd.rz = rotZ;
			rd.rw = rotW;

			strncpy(rd.name, getNodeName(plug.node()).asChar(), sizeof(rd.name));

			g_RotData = rd;
			g_RotDataUpdated = true;

		}

	}
	if (plug.partialName(0, 0, 0, 0, 0, 1) == "scaleX" || plug.partialName(0, 0, 0, 0, 0, 1) == "scaleY" || plug.partialName(0, 0, 0, 0, 0, 1) == "scaleZ")
	{
		MFnTransform transFn(plug.node(), &status);


		double scale[3];

		MStatus test;
		MTransformationMatrix tMtx = transFn.transformation(&test);


		MString msg;
		if (test == MS::kSuccess)
		{
			tMtx.getScale(scale, MSpace::kWorld);

			msg = "Scale data: \n";
			msg += scale[0];
			msg += ", ";
			msg += scale[1];
			msg += ", ";
			msg += scale[2];
			msg += ", ";

			MGlobal::displayInfo(msg);


			ScaleData sd;

			sd.sx = scale[0];
			sd.sy = scale[1];
			sd.sz = scale[2];

			strncpy(sd.name, getNodeName(plug.node()).asChar(), sizeof(sd.name));

			g_ScaleData = sd;
			g_ScaleDataUpdated = true;


		}

	}


	/*
	if (plug.partialName(0, 0, 0, 0, 0, 1) == "translateX")
	{
		if (msg & MNodeMessage::AttributeMessage::kAttributeSet) {

			MGlobal::displayInfo(MString("Transform node changed in x axis"));
		}
	}

	if (plug.partialName(0, 0, 0, 0, 0, 1) == "translateY")
	{
		if (msg & MNodeMessage::AttributeMessage::kAttributeSet) {

			MGlobal::displayInfo(MString("Transform node changed in y axis"));
		}
	}

	if (plug.partialName(0, 0, 0, 0, 0, 1) == "translateZ")
	{
		if (msg & MNodeMessage::AttributeMessage::kAttributeSet) {

			MGlobal::displayInfo(MString("Transform node changed in z axis"));
		}
	}
	*/


	// Get material

	// texture
		MGlobal::displayInfo(MString("--- kFiletexture! ----"));

		float rgbArr[3];

		MStatus meshStatus;
		MFnMesh mesh1 = plug.node(&meshStatus);
		MString meshName = getNodeName(mesh1.parent(0));

		MObjectArray PolygonSets;
		MObjectArray PolygonComponents;
		unsigned int instanceNum = mesh1.dagPath().instanceNumber();

		MObject shaderNode = MObject::kNullObj;
		MObject connectedObject = MObject::kNullObj;

		if (!mesh1.getConnectedSetsAndMembers(instanceNum, PolygonSets, PolygonComponents, true)) {
			//MGlobal::displayError("MFnMesh::getConnectedSetsAndMembers");
			
		}

		MPlug shaderPlug; 
		if (PolygonSets.length() > 0)
		{
			MObject setNode = PolygonSets[0];

			MFnDependencyNode depNode(setNode);

			shaderPlug = depNode.findPlug("surfaceShader");

			if (!shaderPlug.isNull()) {
				MPlugArray connectedPlugs;
				MGlobal::displayInfo(MString("SufaceShader found! --- --- -- - - "));
				if (shaderPlug.connectedTo(connectedPlugs, true, false) && connectedPlugs.length() == 1)
				{
					MGlobal::displayInfo(MString("----- shaderPlug.connectedTo "));

					const MPlug& connectedPlug = connectedPlugs[0];
					shaderNode = connectedPlug.node();
				}
			}
		}



		/*
		MPlugArray mPlugArray1;
		depNode.getConnections(mPlugArray1);

		for (int i = 0; i < mPlugArray1.length(); i++)
		{
			MGlobal::displayInfo(MString("mPlugArray1 name: "));
			MGlobal::displayInfo(mPlugArray1[i].name());

			MPlugArray mPlugArray2;
			mPlugArray1[i].connectedTo(mPlugArray2, 1, 0);


			for (int j = 0; j < mPlugArray2.length(); j++)
			{
				MGlobal::displayInfo(MString("mPlugArray2 name: "));
				MGlobal::displayInfo(mPlugArray2[j].name());
				if (mPlugArray2[j].node().hasFn(MFn::Type::kLambert))
				{
					MGlobal::displayInfo(MString("--- AKJSDHKASH DKAHSD JAHDKA  SDJAHD k! ----"));
					MObject shaderNode = mPlugArray2[j].node();
					cplug = MFnDependencyNode(shaderNode).findPlug("color"); // shader node
				}
			}
		}
		*/


		MPlug cPlug = MFnDependencyNode(shaderNode).findPlug("color");
		if (!cPlug.isNull())
		{
			MPlugArray plugArray;
			if (cPlug.connectedTo(plugArray, true, false) && plugArray.length() == 1)
			{
				MGlobal::displayInfo(MString("----- cplug.connectedTo "));
				// Check for a file texture node connection
				//
				const MPlug& connectedPlug = plugArray[0];
				connectedObject = connectedPlug.node();
				if (connectedObject.hasFn(MFn::kFileTexture))
				{
					// Get the name of the file texture image and acquire a texture to use
					MString fileTextureName;
					MStatus tempStatus;
					MRenderUtil::exactFileTextureName(connectedObject, fileTextureName, &tempStatus);

					MGlobal::displayInfo(MString("----- The texture is: " + fileTextureName));
					MGlobal::displayInfo(MString("----- Name of mesh: " + meshName));
					MGlobal::displayInfo(MString("----- The material name is: " + MFnDependencyNode(shaderNode).name()));		

					MaterialData matData;

					
					strncpy(matData.name, meshName.asChar(), sizeof(matData.name));
					strncpy(matData.texturePath, fileTextureName.asChar(), sizeof(matData.texturePath));
					strncpy(matData.matName, MFnDependencyNode(shaderNode).name().asChar(), sizeof(matData.matName));

					matData.isTexture = true;

					prepareMaterialMessage(matData);
				}
				
			}
			else {
				MFnDependencyNode(shaderNode).findPlug("colorR").getValue(rgbArr[0]);
				MFnDependencyNode(shaderNode).findPlug("colorG").getValue(rgbArr[1]);
				MFnDependencyNode(shaderNode).findPlug("colorB").getValue(rgbArr[2]);

				MString colorStr;
				colorStr = "R: ";
				colorStr += rgbArr[0];
				colorStr += ", G: ";
				colorStr += rgbArr[1];
				colorStr += ", B: ";
				colorStr += rgbArr[2];

				MGlobal::displayInfo(MString("----- Only color found: " + colorStr));

				MaterialData matData;

				matData.rgb[0] = rgbArr[0];
				matData.rgb[1] = rgbArr[1];
				matData.rgb[2] = rgbArr[2];

				strncpy(matData.name, meshName.asChar(), sizeof(matData.name));
				strncpy(matData.matName, MFnDependencyNode(shaderNode).name().asChar(), sizeof(matData.matName));
				matData.isTexture = false;


				prepareMaterialMessage(matData);
			}
		}
	



	/*
	 MStatus status;
        MFnDependencyNode node(object, &status);
        if (status)
        {
            node.findPlug("colorR").getValue(fDiffuseColor[0]);
            node.findPlug("colorG").getValue(fDiffuseColor[1]);
            node.findPlug("colorB").getValue(fDiffuseColor[2]);
            node.findPlug("diffuse").getValue(fDiffuse);
            node.findPlug("transparency").getValue(fTransparency);
            fDiffuseColor[3] = 1.0f - fTransparency;
	*/


	MStatus indexStatus;
	int index = plug.logicalIndex(&indexStatus);

	MStatus meshStatus2;
	MFnMesh mesh = plug.node(&meshStatus2);
	MPoint point;

	float arr[4];

	if (meshStatus2 == MS::kSuccess)
	{
		mesh.getPoint(index, point);
	}


	if (msg == (MNodeMessage::AttributeMessage::kAttributeSet | MNodeMessage::AttributeMessage::kIncomingDirection) && index >= 0)
	{
		MString msg2("Vertex was moved");
		MGlobal::displayInfo(msg2);
		point.get(arr);
		MString p;
		for (int i = 0; i < 4; i++)
		{
			p += arr[i];
			p += " ";
		}
		MGlobal::displayInfo("New value: " + p);
	}
}

void prepareMaterialMessage(MaterialData md) {
	Header h;

	size_t headerSize = sizeof(Header);
	size_t mdSize = sizeof(md);

	size_t localHead = 0;

	h.msgType = MSG_TYPE::Material_Change;
	h.length = mdSize;


	char* data = new char[headerSize + h.length];


	// Header
	memcpy(data, &h, headerSize);
	localHead += headerSize;

	// md
	memcpy(data + localHead, &md, mdSize);


	comLib->send(data, headerSize + h.length);

	delete[] data;
}

void prepareTranslationMessage(TranslationData td) {

	Header h;

	size_t headerSize = sizeof(Header);
	size_t tdSize = sizeof(td);

	size_t localHead = 0;

	h.msgType = MSG_TYPE::Translation;
	h.length = tdSize;


	char* data = new char[headerSize + h.length];


	// Header
	memcpy(data, &h, headerSize);
	localHead += headerSize;

	// Td
	memcpy(data + localHead, &td, tdSize);


	comLib->send(data, headerSize + h.length);

	delete[] data;
}

void prepareScaleMessage(ScaleData sd) {
	Header h;

	size_t headerSize = sizeof(Header);
	size_t sdSize = sizeof(sd);

	size_t localHead = 0;

	h.msgType = MSG_TYPE::Scale;
	h.length = sdSize;


	char* data = new char[headerSize + h.length];


	// Header
	memcpy(data, &h, headerSize);
	localHead += headerSize;

	// Sd
	memcpy(data + localHead, &sd, sdSize);


	comLib->send(data, headerSize + h.length);

	delete[] data;
}

void prepareRotationMessage(RotationData rd) {
	Header h;

	size_t headerSize = sizeof(Header);
	size_t rdSize = sizeof(rd);

	size_t localHead = 0;

	h.msgType = MSG_TYPE::Rotation;
	h.length = rdSize;


	char* data = new char[headerSize + h.length];


	// Header
	memcpy(data, &h, headerSize);
	localHead += headerSize;

	// Rd
	memcpy(data + localHead, &rd, rdSize);


	comLib->send(data, headerSize + h.length);

	delete[] data;
}

void nodeRemoved(MObject &node, void* clientData) {

	if (node.hasFn(MFn::kTransform))
	{
		MStatus status;
		MFnTransform mesh(node, &status);
		if (status == MS::kSuccess)
		{
			MString nodeName = "|";
			nodeName += mesh.name();
			MeshInfo meshinfo;

			strncpy(meshinfo.name, nodeName.asChar(), sizeof(meshinfo.name));

			prepareRemovedMessage(meshinfo);
		}

	}
}

void prepareRemovedMessage(MeshInfo mesh) {
	Header h;

	size_t headerSize = sizeof(Header);
	size_t meshSize = sizeof(mesh);

	size_t localHead = 0;

	h.msgType = MSG_TYPE::Node_Removed;
	h.length = meshSize;


	char* data = new char[headerSize + h.length];


	// Header
	memcpy(data, &h, headerSize);
	localHead += headerSize;

	// md
	memcpy(data + localHead, &mesh, meshSize);


	comLib->send(data, headerSize + h.length);

	delete[] data;
}

// called when the plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	// most functions will use this variable to indicate for errors
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
	}

	MGlobal::displayInfo("Maya plugin loaded! 12345");



	MStatus status = MS::kSuccess;
	MCallbackId id = MTimerMessage::addTimerCallback(1 / fps, TimerCallBack, NULL, &status);

	MStatus status2;
	// Node added
	MCallbackId id2 = MDGMessage::addNodeAddedCallback(NodeAdded, kDefaultNodeType, NULL, &status2);

	// Node removed
	MStatus removedStatus;
	MCallbackId removedId = MDGMessage::addNodeRemovedCallback(nodeRemoved, kDefaultNodeType, NULL, &removedStatus);

	// node name changed
	MStatus nodeNameChangedStatus;
	MObject objNULL;
	MCallbackId nodeChangedId = MNodeMessage::addNameChangedCallback(objNULL, nodeRenamed, NULL, &nodeNameChangedStatus);

	// Node attr change
	MObject objNode;
	MSelectionList list;
	MGlobal::getActiveSelectionList(list);
	list.getDependNode(0, objNode);

	// camera changes

	MStatus camChangedStatus;

	MCallbackId camChangedId = MUiMessage::add3dViewPostRenderMsgCallback("modelPanel4", cameraViewChanged, NULL, &camChangedStatus);




	if (nodeNameChangedStatus == MS::kSuccess)
	{
		if (myCallbackArray.append(nodeChangedId) == MS::kSuccess)
		{

		}
	}

	if (status2 == MS::kSuccess)
	{
		if (myCallbackArray.append(id2) == MS::kSuccess)
		{

		}
	}
	if (removedStatus == MS::kSuccess)
	{
		if (myCallbackArray.append(removedId) == MS::kSuccess)
		{

		}
	}

	if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(id) == MS::kSuccess)
		{

		}
	}

	if (camChangedStatus == MS::kSuccess)
	{
		if (myCallbackArray.append(camChangedId) == MS::kSuccess)
		{

		}
	}


	// if res == kSuccess then the plugin has been loaded, 
	// otherwise is has not.

	comLib = new ComLib();


	return res;
}


EXPORT MStatus uninitializePlugin(MObject obj)
{
	// simply initialize the Function set with the MObject that represents
	// our plugin
	MFnPlugin plugin(obj);

	// if any resources have been allocated, release and free here before
	// returning...
	MMessage::removeCallbacks(myCallbackArray);

	delete comLib;

	MGlobal::displayInfo("Maya plugin unloaded!");

	return MS::kSuccess;
}