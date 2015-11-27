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

//Map Data:
struct mapData
{
	int posX;
	int posZ;
	float rotY;
	string tileType;
	bool walkable;
};