#include "maya_includes.h"
#include "Structs.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <stdlib.h>


using namespace DirectX;
using namespace std;

static const float EPS = 0.1f;

//UI
void initUI();
void deleteUI();

//export
void exportToFile();

class exportLevel : public MPxCommand
{
public:
	exportLevel() {};
	~exportLevel() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("Export Level Called\n");
		MGlobal::displayInfo("Button press!");
		exportToFile();
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

void exportToFile()
{

	MGlobal::displayInfo("Test of button and things");

	MItDag itMeshes(MItDag::kDepthFirst, MFn::kMesh);
	
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

		MGlobal::displayInfo(MString() + meshTemp.name());

		if (transformTemp.hasAttribute("tileType"))
		{
			//find type of tile
			MFnEnumAttribute tileEnum = transformTemp.attribute("tileType");
			MGlobal::displayInfo("Has tiletype attr");

			MString tileType = tileEnum.fieldName(0);

			if (!strcmp(tileType.asChar(), "floorTile"))
			{
				MGlobal::displayInfo("tile says " + tileType);
			}


		}
		if (transformTemp.hasAttribute("walkable"))
		{
			MFnEnumAttribute walkEnum = transformTemp.attribute("walkable");
			MGlobal::displayInfo("has walkable attr");


			MString answer = walkEnum.fieldName(0);

			if (!strcmp(answer.asChar(), "Yes"))
			{
				MGlobal::displayInfo("walkable: yes");
			}
			if (!strcmp(answer.asChar(), "No"))
			{
				MGlobal::displayInfo("walkable: no");
			}


		}
		if (transformTemp.hasAttribute("entrance"))
		{
			MFnEnumAttribute entranceEnum = transformTemp.attribute("entrance");
			MGlobal::displayInfo("has entrance attr");

			MString answer = entranceEnum.fieldName(0);


			if (!strcmp(answer.asChar(), "Yes"))
			{
				MGlobal::displayInfo("entrance: yes");
			}
			if (!strcmp(answer.asChar(), "No"))
			{
				MGlobal::displayInfo("entrance: no");
			}

		}
		if (transformTemp.hasAttribute("goal"))
		{
			MFnEnumAttribute goalEnum = transformTemp.attribute("goal");
			MGlobal::displayInfo("has goal attr");
			MString answer = goalEnum.fieldName(0);


			if (!strcmp(answer.asChar(), "Yes"))
			{
				MGlobal::displayInfo("goal: yes");
			}
			if (!strcmp(answer.asChar(), "No"))
			{
				MGlobal::displayInfo("goal: no");
			}
		}	

		//Name
		MString name = meshTemp.name();

		MGlobal::displayInfo(MString() + "Name: " + name);

		//Translate, tile and rotate so we can get the tile position and render object position
		MFloatVector tPosition;
		XMFLOAT3 TranslateTransfer;

		tPosition = transformTemp.translation(MSpace::kTransform);

		TranslateTransfer.x = tPosition.x;
		TranslateTransfer.y = tPosition.y;
		TranslateTransfer.z = tPosition.z;

		int coordX = (int)(tPosition.x + EPS);
		int coordY = (int)(tPosition.y + EPS);

		if (coordX < 0 || coordY < 0)
		{
			cout << "A gameObject:" + name + " is out of bounds.";
		}

		MEulerRotation eulerRotation;
		transformTemp.getRotation(eulerRotation);

		MGlobal::displayInfo(MString() + "Translation: " + TranslateTransfer.x + " " + TranslateTransfer.y + " " + TranslateTransfer.z);
		MGlobal::displayInfo(MString() + "Tiles: " + coordX + " " + coordY);
		MGlobal::displayInfo(MString() + "Rotation: " + eulerRotation.x + " " + eulerRotation.y + " " + eulerRotation.z);

		itMeshes.next();
	}
}

