#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <string>
using namespace std;
#define BUFF_SIZE 1 * 1 << 20
class comLib
{

private:
	HANDLE hMapFile;
	HANDLE hMapFileHeadTail;
	char* pBuf;
	size_t* head;
	size_t* tail;
	size_t* sharedHeadTail;


	void openFileMap();
	void openHeadTailFileMap();
	bool fileMapFound;

public:
	comLib();
	~comLib();

	bool test();

};

