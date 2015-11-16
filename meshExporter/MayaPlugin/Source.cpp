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

class exporterCommands : public MPxCommand
{
public:
	exporterCommands() {};
	~exporterCommands() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("exporterCommand Called\n");
		MGlobal::displayInfo("Button press!");
		ExportEverything();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new exporterCommands;
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
	// register our command with maya
	status = plugin.registerCommand("exporterCommands", exporterCommands::creator);

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



	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}


void initUI()
{

	MGlobal::executeCommand("window -title ""MeshExporter"" -w 200 -h 200;");
	MGlobal::executeCommand("columnLayout;");
	MGlobal::executeCommand("button -w 50 -h 20 -label ""Export"" -command ""exporterCommands"";");
	MGlobal::executeCommand("showWindow;;");

}

void ExportEverything()
{
	MGlobal::displayInfo("Pretending To Export Everything!!");
}
