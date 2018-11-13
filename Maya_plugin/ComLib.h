#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#define BUFF_SIZE 1 * 1 << 20
class ComLib
{

private:
	HANDLE hMapFile;
	HANDLE hMapFileHeadTail;
	char* pBuf;
	size_t* head;
	size_t* tail;
	size_t* sharedHeadTail;


	void createFileMap();
	void createHeadTailFileMap();
public:
	ComLib();
	~ComLib();


	void test();

	void send(const void * data, size_t length);
};

struct Header {
	int msgType;
	int length;
};