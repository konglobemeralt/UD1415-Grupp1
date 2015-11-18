#include "maya_includes.h"
#include "Structs.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <string>
#include <vector>


using namespace DirectX;
using namespace std;

//UI
void initUI();
void deleteUI();

//export
void ExportFinder(bool sl);
bool ExportMesh(MFnDagNode &primaryMeshDag);

class exportAll : public MPxCommand
{
public:
	exportAll() {};
	~exportAll() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("exportAll Called\n");
		MGlobal::displayInfo("Button press!");
		ExportFinder(false);
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
		ExportFinder(true);
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

	deleteUI();
	
	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}


void initUI()
{
	MGlobal::executeCommand("window -wh 200 100 -s false -title ""MeshExporter"" ""meshExporterUI"";");
	MGlobal::executeCommand("columnLayout -columnAttach ""both"" 5 -rowSpacing 5 -columnWidth 100;;");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Everything"" -command ""exportAll"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Selected"" -command ""exportSelected"";");
	MGlobal::executeCommand("showWindow;;");
	MGlobal::executeCommand("window -e -wh 200 100 ""meshExporterUI"";");

	
	MGlobal::displayInfo("UI created");
}

void deleteUI()
{
	MGlobal::executeCommand("deleteUI -window ""meshExporterUI"";");
	MGlobal::displayInfo("UI deleted");
}

void ExportFinder(bool sl)
{

	MStringArray scene;
	if (sl)
	{
		MGlobal::executeCommand("ls -sl", scene);
		MGlobal::displayInfo("Pretending To Export Selected!!");
	}
	else
	{
		MGlobal::executeCommand("ls", scene);
		MGlobal::displayInfo("Pretending To Export Everything!!");
	}

	MDagPath dag_path;
	MItDag dag_iter(MItDag::kBreadthFirst, MFn::kMesh);
	while (!dag_iter.isDone())
	{
		if (dag_iter.getPath(dag_path))
		{
			MFnDagNode dag_node = dag_path.node();
			MFnDagNode transform = dag_node.parent(0);
			MFnDagNode parentPath(transform.parent(0));

			if (strcmp(parentPath.fullPathName().asChar(), ""))
				break;

			if (!dag_node.isIntermediateObject())
				for (int i = 0; i < scene.length(); i++)
					if (strcmp(transform.name().asChar(), scene[i].asChar()) == 0)
					{
						ExportMesh(transform);
					}
		}
		dag_iter.next();
	}
}

void ExtractLights(MFnDagNode &meshDag, Geometry &geometry)
{

}

Geometry ExtractGeometry(MFnDagNode &meshDag)
{
	Geometry geometry;
	ExtractLights(meshDag, geometry);




	return geometry;
}

Material ExtractMaterial(MFnDagNode &meshDag)
{
	Material material;




	return material;
}

bool ExportMesh(MFnDagNode &primaryMeshDag)
{

	Mesh primaryMesh;
	primaryMesh.Name = primaryMeshDag.name().asChar();
	primaryMesh.geometry = ExtractGeometry(primaryMeshDag);
	primaryMesh.material = ExtractMaterial(primaryMeshDag);

	//skeletonID?

	MItDag subMeshes(MItDag::kBreadthFirst, MFn::kMesh);
	subMeshes.reset(primaryMeshDag.object(), MItDag::kBreadthFirst, MFn::kMesh);

	MDagPath dag_path;
	while (!subMeshes.isDone())
	{
		if (subMeshes.getPath(dag_path))
		{
			MFnDagNode dag_node = dag_path.node();
			MFnDagNode transform = dag_node.parent(0);
			MFnDagNode parentPath(transform.parent(0));


			MGlobal::displayInfo(parentPath.fullPathName());
			MGlobal::displayInfo(primaryMeshDag.fullPathName());

			if (!strcmp(parentPath.fullPathName().asChar(), primaryMeshDag.fullPathName().asChar()))
				if (!dag_node.isIntermediateObject())
					{
						MGlobal::displayInfo(dag_node.name());

						SubMesh subMesh;
						subMesh.Name = dag_node.name().asChar();
						subMesh.geometry = ExtractGeometry(dag_node);

						primaryMesh.subMeshes.push_back(subMesh);

					}
		}
		subMeshes.next();
	}

	return false;


}