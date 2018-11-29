#pragma once
#include <vector>

using namespace std;

struct Vertex {
	float x;
	float y;
	float z;
};

struct UV {
	float x;
	float y;
};

struct Normal {
	float x;
	float y;
	float z;
};


struct MeshInfo {

	int nrOfVertices;
	int nrOfTriVertices;

	int msgType;
};

struct Header {
	int msgType;
	int length;
};

struct vectorData {
	vector<Vertex> v;
	vector<int> vertexIndices;
	vector<int> normalIndices;
	vector<UV> uv;
	vector<Normal> normal;
};