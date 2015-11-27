#include "maya_includes.h"
#include "Structs.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <shlobj.h>
#include <direct.h>
#include <string>
#include <vector>
#include <stdlib.h>
// basic file operations

using namespace DirectX;
using namespace std;

static const float EPS = 0.1f;

//UI
void initUI();
void deleteUI();

//export
void exportLevelData();
void exportToFile(std::vector<std::string> formattedLevelData);

class exportLevel : public MPxCommand
{
public:
	exportLevel() {};
	~exportLevel() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("Export Level Called\n");
		MGlobal::displayInfo("Button press!");
		exportLevelData();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new exportLevel;
	}

};

// called when the plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	// most functions will use this variable to indicate for errors
	MStatus res = MS::kSuccess;

	MFnPlugin plugin(obj, "LevelExporter", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
	}

	MStatus status;
	// register our commands with maya
	status = plugin.registerCommand("exportLevel", exportLevel::creator);

	initUI();

	MGlobal::displayInfo("Maya plugin loaded!!");

	return res;
}


EXPORT MStatus uninitializePlugin(MObject obj)
{
	// simply initialize the Function set with the MObject that represents
	// our plugin
	MFnPlugin plugin(obj);

	// if any resources have been allocated, release and free here before
	// returning...

	MStatus status;
	// deregister our commands with maya
	status = plugin.deregisterCommand("exportLevel");

	deleteUI();

	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}


void initUI()
{
	MGlobal::executeCommand("window -wh 250 105 -s true -title ""LevelExporter"" ""levelExporterUI"";");
	MGlobal::executeCommand("columnLayout -columnAttach ""left"" 5 -rowSpacing 5 -columnWidth 100;;");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Level"" -command ""exportLevel"";");

	MGlobal::executeCommand("showWindow;");

	MGlobal::displayInfo("UI created");
}

void deleteUI()
{
	MGlobal::executeCommand("deleteUI -window ""levelExporterUI"";");
	MGlobal::displayInfo("UI deleted");
}

void exportLevelData()
{
	//Initalizing-----------------------------------
	std::vector<std::string> formattedLevelData;
	formattedLevelData.clear();

	std::string formattedOutput;

	MGlobal::displayInfo("Export level");

	MItDag itMeshes(MItDag::kDepthFirst, MFn::kMesh);

	//Level Header
	float version = 1.0;
	int levelSIzeX = 0;
	int levelSIzeY = 0;

	//mapData
	std::vector<mapData> mData;

	//Do work----------------------------------------

	while (!itMeshes.isDone())
	{
		MFnMesh meshTemp = itMeshes.currentItem();
		MFnTransform transformTemp = meshTemp.parent(0);

		//
		//MObject test = transformTemp.attribute("tileType");
		//MFn::kEnumAttribute;
		//MFnEnumAttribute enumTest = test;
		//MString attributeName = enumTest.name();

		//MGlobal::displayInfo(attributeName);

		//Name
		MString Meshname = meshTemp.name();

		MGlobal::displayInfo(MString() + "Name: " + Meshname);

		std::string tname = Meshname.asChar();

		//Translate/tile
		MFloatVector tPosition;

		tPosition = transformTemp.translation(MSpace::kTransform);

		int coordX = (int)(tPosition.x + EPS);
		int coordZ = (int)(tPosition.z + EPS);

		if (coordX < 0 || coordZ < 0)
		{
			cout << "A gameObject:" + Meshname + " is out of bounds.";
		}

		//Grid size
		if (levelSIzeX < coordX)
			levelSIzeX = coordX;
		if (levelSIzeY < coordZ)
			levelSIzeY = coordZ;

		//Rotation
		MEulerRotation eulerRotation;
		transformTemp.getRotation(eulerRotation);

		//MGlobal::displayInfo(MString() + meshTemp.name());
		//MGlobal::displayInfo(MString() + "Translation: " + tPosition.x + " " + tPosition.y + " " + tPosition.z);
		//MGlobal::displayInfo(MString() + "Tiles: " + coordX + " " + coordY);
		//MGlobal::displayInfo(MString() + "Rotation: " + eulerRotation.x + " " + eulerRotation.y + " " + eulerRotation.z);

		//Mapping the data
		mData.push_back(mapData());
		mData.back().modelID = tname;
		mData.back().posX = coordX;
		mData.back().posZ = coordZ;
		mData.back().eulerRotation = eulerRotation;

		//Extracting enums
		if (transformTemp.hasAttribute("tileType"))
		{
			//find type of tile
			MFnEnumAttribute tileEnum = transformTemp.attribute("tileType");
			MGlobal::displayInfo("Has tiletype attr");

			MString tileType = tileEnum.fieldName(0);

			if (!strcmp(tileType.asChar(), "floorTile"))
			{
				MGlobal::displayInfo("tile says " + tileType);
				mData.back().tileType = tileType.asChar();
			}
		}
		if (transformTemp.hasAttribute("walkable"))
		{
			//find if walkable or not
			MFnEnumAttribute walkEnum = transformTemp.attribute("walkable");
			MGlobal::displayInfo("has walkable attr");

			MString answer = walkEnum.fieldName(0);

			if (!strcmp(answer.asChar(), "Yes"))
			{
				MGlobal::displayInfo("walkable: yes");
				mData.back().walkable = true;
			}
			if (!strcmp(answer.asChar(), "No"))
			{
				MGlobal::displayInfo("walkable: no");
				mData.back().walkable = false;
			}
		}
		if (transformTemp.hasAttribute("entrance"))
		{
			//find if entrance or not
			MFnEnumAttribute entranceEnum = transformTemp.attribute("entrance");
			MGlobal::displayInfo("has entrance attr");

			MString answer = entranceEnum.fieldName(0);

			if (!strcmp(answer.asChar(), "Yes"))
			{
				MGlobal::displayInfo("entrance: yes");
				mData.back().entrance = true;
			}
			if (!strcmp(answer.asChar(), "No"))
			{
				MGlobal::displayInfo("entrance: no");
				mData.back().entrance = false;
			}
		}
		if (transformTemp.hasAttribute("goal"))
		{
			//find if goal or not
			MFnEnumAttribute goalEnum = transformTemp.attribute("goal");
			MGlobal::displayInfo("has goal attr");
			MString answer = goalEnum.fieldName(0);

			if (!strcmp(answer.asChar(), "Yes"))
			{
				MGlobal::displayInfo("goal: yes");
				mData.back().goal = true;
			}
			if (!strcmp(answer.asChar(), "No"))
			{
				MGlobal::displayInfo("goal: no");
				mData.back().goal = false;
			}
		}

		itMeshes.next();
	}

	//Translating and exporting--------------------------
	formattedOutput += "Version,";
	formattedOutput += std::to_string(version);
	formattedOutput += "\nlevelSizeX," + std::to_string(levelSIzeX);
	formattedOutput += "\nlevelSizeY," + std::to_string(levelSIzeY);
	for (auto i : mData)
	{
		formattedOutput += "\n";
		formattedOutput += std::to_string(i.posX) + "," + std::to_string(i.posZ);
		formattedOutput += ",";
		formattedOutput += std::to_string(i.eulerRotation.y);
		formattedOutput += "," + i.tileType;
		formattedOutput += "," + std::to_string(i.walkable);
		formattedOutput += "," + std::to_string(i.entrance);
		formattedOutput += "," + std::to_string(i.goal);
	}

	formattedLevelData.push_back(formattedOutput);
	exportToFile(formattedLevelData);
}

void exportToFile(std::vector<std::string> formattedLevelData)
{
	string outputPath;
	string levelName = "level1.lvl";

	char userPath[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userPath);
	outputPath = (string)userPath + "\\Desktop\\ExportedLevels";
	ofstream outputFile;
	_mkdir(outputPath.c_str());
	outputPath += "\\" + levelName;

	outputFile.open(outputPath, ios::out);

	for (std::string formattedOutput : formattedLevelData)
	{
		outputFile << formattedOutput << std::endl;
	}

	outputFile.close();
}

