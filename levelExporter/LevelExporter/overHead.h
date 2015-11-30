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
	int levelSIzeX;
	int levelSIzeY;
	int nrOfTileObjects;
};

//Map Data:
struct mapData
{
	int posX;
	int posZ;
	float rotY;
	//Change comment of tileType
	//string tileType
	int tileType;
	bool walkable;
};