#include "ComLib.h"



void ComLib::createFileMap()
{

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUFF_SIZE, L"myMap");

	//first = (GetLastError() != ERROR_ALREADY_EXISTS);
	// Check
	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d). \n"), GetLastError());
	}


	pBuf = (char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFF_SIZE);

	// Check
	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d). \n"), GetLastError());
		CloseHandle(hMapFile);

	}
}

void ComLib::createHeadTailFileMap()
{

	hMapFileHeadTail = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8, L"HeadTail");

	// Check
	if (hMapFileHeadTail == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d). \n"), GetLastError());
	}


	sharedHeadTail = (size_t*)MapViewOfFile(hMapFileHeadTail, FILE_MAP_ALL_ACCESS, 0, 0, 8);

	// Check
	if (sharedHeadTail == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d). \n"), GetLastError());
		CloseHandle(hMapFileHeadTail);
	}
}

ComLib::ComLib()
{	
	createFileMap();

	createHeadTailFileMap();

	head = (size_t*)sharedHeadTail;
	tail = (size_t*)sharedHeadTail + 1;
}


ComLib::~ComLib()
{

	CloseHandle(hMapFile);
	CloseHandle(hMapFileHeadTail);
}

void ComLib::test()
{
	char* msg = new char[8];
	memset(msg, '\0', 8);

	msg[0] = 'H';
	msg[1] = 'E';
	msg[2] = 'L';
	msg[3] = 'L';
	msg[4] = 'O';
	msg[5] = 'O';
	msg[6] = 'O';

	memcpy(pBuf, msg, 8);

	*head += 8;

	delete[] msg;
}

void ComLib::send(const void * data, size_t length)
{

	//Header h;
	//h.length = length; // msg length
	//size_t headerSize = sizeof(h);

	//memcpy(pBuf, &h, headerSize);

	//*head += headerSize;

	memcpy(pBuf + *head, data, length);

	*head += length;

}