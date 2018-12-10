#pragma once
#include <vector>

using namespace std;

 enum MSG_TYPE {
	MeshAdded,
	Translation,
	Scale,
	Rotation
};

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

struct ScaleData {
	double sx;
	double sy;
	double sz;
	char name[50];
};

struct RotationData {
	double rx;
	double ry;
	double rz;
	double rw;
	char name[50];
};

struct MeshInfo {
	char name[50];
	int nrOfVertices;
	int nrOfTriVertices;
	int nrOfNormals;
	int msgType;
};

struct Header {
	MSG_TYPE msgType;
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