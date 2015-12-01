#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <shlobj.h>
#include <direct.h>
#include <stdlib.h>

using namespace std;

struct levelHeader
{
	int version;
	int levelSizeX;
	int levelSizeY;
	int nrOfTileObjects;
};

//Map Data:
struct mapData
{
	int posX;
	int posZ;
	float rotY;
	int tileType;
	
};

//Map Data for txt files:
struct mapDataString
{
	int posX;
	int posZ;
	float rotY;
	string tileType;
};