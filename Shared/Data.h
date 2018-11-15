#pragma once
#include <vector>

using namespace std;

struct Vertex {
	float x;
	float y;
	float z;
};


struct MeshInfo {
	vector<Vertex> vertices;
	int nrOfVertices;
	int nrOfTriVertices;
	vector<int> indices;
	int msgType;
};

struct Header {
	int msgType;
	int length;
};
