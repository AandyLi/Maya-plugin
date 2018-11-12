// UD1414_Plugin.cpp : Defines the exported functions for the DLL application.

#include "Maya_Includes.h"
#include <iostream>
#include <string>
#include "ComLib.h"
#include "Data.h"
#include <vector>

using namespace std;
MCallbackIdArray myCallbackArray;
void nodeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData);
void meshAdded(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData);

MCallbackId meshNodeCallbackID;

//ComLib* comLib;

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
	MString msg("Elapsd time: ");
	msg += elapsedTime;
	MGlobal::displayInfo(msg);
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

	// test change
	
	if (status == MS::kSuccess)
	{
		MGlobal::displayInfo(MString("Status  ==  Succsess ") + getNodeName(plug.node()));

		MeshInfo mesh;

		MFloatPointArray pts;
		fn.getPoints(pts);

		MString points;


		// Debug
		for (int i = 0; i < pts.length(); i++)
		{
			points += "Vertex ";
			points += i;
			points += "::: x: ";
			points += pts[i].x;
			points += ", y: ";
			points += pts[i].y;
			points += ", z: ";
			points += pts[i].z;
			points += "\n";
		}

		MGlobal::displayInfo(points);

		// Get all vertices
		for (int i = 0; i < pts.length(); i++)
		{
			Vertex vtx;
			vtx.x = pts[i].x;
			vtx.y = pts[i].y;
			vtx.z = pts[i].z;

			mesh.vertices.push_back(vtx);
		}

		mesh.nrOfVertices = pts.length();



		MIntArray triangleCounts;
		MIntArray triangleVertices;

		fn.getTriangles(triangleCounts, triangleVertices);


		int* triCountsArr = new int[triangleCounts.length()];
		triangleCounts.get(triCountsArr);

		int* triVertsArr = new int[triangleVertices.length()]; // important!
		triangleVertices.get(triVertsArr);

		mesh.nrOfTriVertices = triangleVertices.length();

		// Store indices in vector
		for (int i = 0; i < triangleVertices.length(); i++)
		{
			mesh.indices.push_back(triangleVertices[i]);
		}

		// triangleCounts
		MString msg(getNodeName(plug.node()) + ": Triangle counts array length  " + triangleCounts.length());
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": Triangle counts array ");
		for (int i = 0; i < triangleCounts.length(); i++)
		{
			msg += triangleCounts[i];
			msg += ", ";
		}
		MGlobal::displayInfo(msg);

		// triangleVertices important
		msg = (getNodeName(plug.node()) + ": Triangle vertices array length  " + triangleVertices.length());
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": Triangle vertices array ");
		for (int i = 0; i < triangleVertices.length(); i++)
		{
			msg += triangleVertices[i];
			msg += ", ";
		}

		MGlobal::displayInfo(msg);

		delete[] triVertsArr;
		delete[] triCountsArr;

		// remove callback(s)
		MNodeMessage::removeCallback(meshNodeCallbackID);

		//myCallbackArray.remove(meshNodeCallbackID);
		MGlobal::displayInfo(MString("Callback removed"));
	}
}

void nodeRenamed(MObject &node, const MString &str, void *clientData) {
	MString nodeName = getNodeName(node);
	MString msg(str + " has been renamed to " + nodeName);
	MGlobal::displayInfo(msg);
}

void nodeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void *clientData) {

	MGlobal::displayInfo(MString("Node changed function:"));

	MStatus status;
	MFnMesh fn(plug.node(), &status);
	// MFnMesh fn = plug.node(&status); Not working!

	/*
	if (status == MS::kSuccess)
	{
		MGlobal::displayInfo(MString("Status  ==  Succsess ") + plug.node().apiTypeStr() + msg);

		MFloatPointArray pts;
		fn.getPoints(pts);

		MString points;

		for (int i = 0; i < pts.length(); i++)
		{
			points += "Vertex ";
			points += i;
			points += "::: x: ";
			points += pts[i].x;
			points += ", y: ";
			points += pts[i].y;
			points += ", z: ";
			points += pts[i].z;
			points += "\n";
		}

		MGlobal::displayInfo(points);

		MIntArray triangleCounts;
		MIntArray triangleVertices;

		fn.getTriangles(triangleCounts, triangleVertices);

		int* triCountsArr = new int[triangleCounts.length()];
		triangleCounts.get(triCountsArr);

		int* triVertsArr = new int[triangleVertices.length()];
		triangleVertices.get(triVertsArr);


		// triangleCounts
		MString msg(getNodeName(plug.node()) + ": Triangle counts array length  " + triangleCounts.length());
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": Triangle counts array ");
		for (int i = 0; i < triangleCounts.length(); i++)
		{
			msg += triCountsArr[i];
			msg += ", ";
		}
		MGlobal::displayInfo(msg);

		// triangleVertices
		msg = (getNodeName(plug.node()) + ": Triangle vertices array length  " + triangleVertices.length());
		MGlobal::displayInfo(msg);

		msg = (getNodeName(plug.node()) + ": Triangle vertices array ");
		for (int i = 0; i < triangleVertices.length(); i++)
		{
			msg += triangleVertices[i];
			msg += ", ";
		}

		MGlobal::displayInfo(msg);

		delete[] triVertsArr;
		delete[] triCountsArr;
	}
	*/


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



	MStatus indexStatus;
	int index = plug.logicalIndex(&indexStatus);

	MStatus meshStatus;
	MFnMesh mesh = plug.node(&meshStatus);
	MPoint point;

	float arr[4];

	if (meshStatus == MS::kSuccess)
	{
		mesh.getPoint(index, point);
	}


	if (msg == MNodeMessage::AttributeMessage::kAttributeSet | MNodeMessage::AttributeMessage::kIncomingDirection && index >= 0)
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

	/*MStatus status = MS::kSuccess;
	MCallbackId id = MTimerMessage::addTimerCallback(5, TimerCallBack, NULL, &status);*/

	MStatus status2;
	// Node added
	MCallbackId id2 = MDGMessage::addNodeAddedCallback(NodeAdded, kDefaultNodeType, NULL, &status2);

	// node name changed
	MStatus nodeNameChangedStatus;
	MObject objNULL;
	MCallbackId nodeChangedId = MNodeMessage::addNameChangedCallback(objNULL, nodeRenamed, NULL, &nodeNameChangedStatus);

	// Node attr change
	MObject objNode;
	MSelectionList list;
	MGlobal::getActiveSelectionList(list);
	list.getDependNode(0, objNode);





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

	/*if (status == MS::kSuccess)
	{
		if (myCallbackArray.append(id) == MS::kSuccess)
		{

		}
	}*/
	// if res == kSuccess then the plugin has been loaded, 
	// otherwise is has not.

	//comLib = new ComLib();


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

	//delete comLib;

	MGlobal::displayInfo("Maya plugin unloaded!");

	return MS::kSuccess;
}