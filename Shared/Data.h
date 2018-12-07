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

struct TranslationData {
	float tx;
	float ty;
	float tz;
	char name[50];
};

struct RotationData {
	double rx;
	double ry;
	double rz;
};

struct ScaleData {
	double sx;
	double sy;
	double sz;
};

struct MeshInfo {
	char name[50];
	int nrOfVertices;
	int nrOfTriVertices;
	int nrOfNormals;
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
	vector<int> uvIndices;
	vector<UV> uv;
	vector<Normal> normal;
};