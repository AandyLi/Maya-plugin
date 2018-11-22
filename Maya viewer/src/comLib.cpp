#include "comLib.h"




comLib::comLib()
{
	fileMapFound = true;
	// Open primary buffer
	openFileMap();

	// Keep track of head and tail using a shared memory location
	openHeadTailFileMap();

	// Assign head and tail to shared memory

	head = (size_t*)sharedHeadTail;
	tail = (size_t*)sharedHeadTail + 1;


}

void comLib::openFileMap()
{
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"myMap");

	// Check
	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping obj (%d). \n"), GetLastError());
		//return 1;
	}

	pBuf = (char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFF_SIZE);

	// Check
	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not open view of file (%d). \n"), GetLastError());

		CloseHandle(hMapFile);
	}
}

void comLib::openHeadTailFileMap()
{
	hMapFileHeadTail = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"HeadTail");


	// Check
	if (hMapFileHeadTail == NULL)
	{
		_tprintf(TEXT("Could not open file mapping obj (%d). \n"), GetLastError());
		fileMapFound = false;
	}


	// Since size_t is 4 bytes we give it double the space for the second size_t (tail)
	sharedHeadTail = (size_t*)MapViewOfFile(hMapFileHeadTail, FILE_MAP_ALL_ACCESS, 0, 0, 8);

	// Check
	if (sharedHeadTail == NULL)
	{
		_tprintf(TEXT("Could not open view of file (%d). \n"), GetLastError());

		CloseHandle(hMapFileHeadTail);
		//return 1;
	}
}

comLib::~comLib()
{
}

bool comLib::test()
{
	if (fileMapFound)
	{

		if (*tail != *head)
		{
			char* msg = new char[8];
			memset(msg, '\0', 8);

			memcpy(msg, pBuf, 8);

			if ((string)msg == "HELLOOO") // Gör om msg till char?
			{
				return true;
			}
		}
	}

	return false;
}

void comLib::recieve(void* data)
{
	if (fileMapFound)
	{

		if (*tail != *head)
		{

			// get header data
			Header h;
			size_t headerSize = sizeof(h);

			memcpy(&h, pBuf + *tail, headerSize);

			////debug
			//MeshInfo m;

			//memcpy(&m, pBuf + headerSize, sizeof(MeshInfo));

			//vectorData vD;
			//vD.v = new Vertex[m.nrOfVertices];
			//vD.indices = new int[m.nrOfTriVertices];
			//memcpy(&vD, pBuf + headerSize + sizeof(MeshInfo), sizeof(vectorData));

			//vector<Vertex> vt;

			//Vertex v;
			//for (int i = 0; i < m.nrOfVertices; i++)
			//{
			//	// vector content (Vertex) has to be added back into a new vector
			//	memcpy(&v, pBuf + headerSize + sizeof(MeshInfo) + (sizeof(Vertex) * i), sizeof(Vertex));
			//	vt.push_back(v);
			//}

			// copy data and header from pBuf to data
			memcpy(data, pBuf + *tail, h.length + headerSize);

			*tail += h.length + headerSize;
		}
	} 
	
	// retry
	retryToGetFileMap();
}

void comLib::retryToGetFileMap()
{
	fileMapFound = true;
	// Open primary buffer
	openFileMap();

	// Keep track of head and tail using a shared memory location
	openHeadTailFileMap();

	// Assign head and tail to shared memory

	head = (size_t*)sharedHeadTail;
	tail = (size_t*)sharedHeadTail + 1;
}