#pragma once
#include <vector>

//Map Data:
struct mapData
{
	string modelID;
	int posX;
	int posZ;
	MEulerRotation eulerRotation;
	string tileType;
	bool walkable;
	bool entrance;
	bool goal;
};