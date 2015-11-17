#include "maya_includes.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <vector>


using namespace DirectX;
using namespace std;

void initUI();

void ExportEverything();
void ExportSelected();

class exportAll : public MPxCommand
{
public:
	exportAll() {};
	~exportAll() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("exportAll Called\n");
		MGlobal::displayInfo("Button press!");
		ExportEverything();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new exportAll;
	}

};


class exportSelected : public MPxCommand
{
public:
	exportSelected() {};
	~exportSelected() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("exportSelected Called\n");
		MGlobal::displayInfo("Second Button press!");
		ExportSelected();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new exportSelected;
	}

};


// called when the plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	// most functions will use this variable to indicate for errors
	MStatus res = MS::kSuccess;


	MFnPlugin plugin(obj, "MeshExporter", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
	}

	MStatus status;
	// register our commands with maya
	status = plugin.registerCommand("exportAll", exportAll::creator);
	status = plugin.registerCommand("exportSelected", exportSelected::creator);

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
	status = plugin.deregisterCommand("exportAll");
	status = plugin.deregisterCommand("exportSelected");

	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}


void initUI()
{

	MGlobal::executeCommand("window -title ""MeshExporter"" -w 300 -h 300;");
	MGlobal::executeCommand("columnLayout;");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Everything"" -command ""exportAll"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Selected"" -command ""exportSelected"";");
	MGlobal::executeCommand("showWindow;;");

}

void ExportEverything()
{
	MGlobal::displayInfo("Pretending To Export Everything!!");


}



void ExportSelected()
{
	MGlobal::displayInfo("Pretending To Export Selected!!");


}