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
void ExtractLights(MFnMesh &mesh, Geometry &geometry);
Geometry ExtractGeometry(MFnMesh &mesh);
Material ExtractMaterial(MFnMesh &mesh);
bool ExportMesh(MFnDagNode &primaryMeshDag);
void ExportFile(Mesh &mesh, std::string path);

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
	MGlobal::executeCommand("window -wh 250 105 -s true -title ""MeshExporter"" ""meshExporterUI"";");
	MGlobal::executeCommand("columnLayout -columnAttach ""left"" 5 -rowSpacing 5 -columnWidth 100;;");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Everything"" -command ""exportAll"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Selected"" -command ""exportSelected"";");
	
	
	MGlobal::executeCommand("showWindow;");
	
	MGlobal::displayInfo("UI created");
}

void deleteUI()
{
	MGlobal::executeCommand("deleteUI -window ""meshExporterUI"";");
	MGlobal::displayInfo("UI deleted");
}

void ExportFinder(bool sl)//sl(selected) sätts genom knapparnas call till ExportFinder
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

	//hitta alla meshes i scenen, kolla mot objekt i listan och om de ligger på toppnivå. Kalla ExportMesh på alla som klarar
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

void ExtractLights(MFnMesh &meshDag, Geometry &geometry)
{
	//TODO
}

Geometry ExtractGeometry(MFnMesh &mesh)
{
	//samlar data om geometrin och sparar i ett Geometryobjekt
	Geometry geometry;
	ExtractLights(mesh, geometry);
	MSpace::Space world_space = MSpace::kTransform;

	MFloatPointArray points;
	MFloatVectorArray normals;

	MGlobal::executeCommand(MString("polyTriangulate -ch 1 " + mesh.name()));//TODO - Bör dra från origmesh eller göra en undo

	mesh.getPoints(points, world_space);
	for (int i = 0; i < points.length(); i++)
	{
		Point temppoints = { points[i].x, points[i].y, -points[i].z };
		geometry.points.push_back(temppoints);
	}

	mesh.getNormals(normals, world_space);
	for (int i = 0; i < normals.length(); i++)
	{
		Normal tempnormals = { normals[i].x, normals[i].y, -normals[i].z };
		geometry.normals.push_back(tempnormals);
	}

	MStringArray uvSets;
	mesh.getUVSetNames(uvSets);

	MFloatArray Us;
	MFloatArray Vs;
	TexCoord UVs;

	MString currentSet = uvSets[0];
	mesh.getUVs(Us, Vs, &currentSet);
	for (int a = 0; a < Us.length(); a++)
	{
		UVs.u = Us[a];
		UVs.v = 1 - Vs[a];
		geometry.texCoords.push_back(UVs);
	}

	MItMeshPolygon itFaces(mesh.dagPath());
	while (!itFaces.isDone()) {
		Face tempface;
		int vc = itFaces.polygonVertexCount();

		for (int i = 0; i < vc; ++i)
		{
			tempface.verts[i].pointID = itFaces.vertexIndex(i);
			tempface.verts[i].normalID = itFaces.normalIndex(i);
			itFaces.getUVIndex(i, tempface.verts[i].texCoordID, &uvSets[0]);
		}

		geometry.faces.push_back(tempface);
		itFaces.next();
	}
	
	return geometry;
}

Material ExtractMaterial(MFnMesh &meshDag)
{
	Material material;

	//hittar alla kopplade surfaceShaders och samlar diffuse och specular(om tillgängligt) färg samt texturfilsnamn
	MObjectArray shaders;
	MIntArray shaderindices;
	meshDag.getConnectedShaders(0, shaders, shaderindices);
	MObject shader;
	int t = shaders.length();
	for (int i = 0; i < t; i++) {
		MPlugArray connections;
		MFnDependencyNode shaderGroup(shaders[i]);
		MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
		shaderPlug.connectedTo(connections, true, false);
		for (uint u = 0; u < connections.length(); u++)
		{
			shader = connections[u].node();
			if (shader.hasFn(MFn::kBlinn))
			{
				MFnBlinnShader fn(shader);
				MPlug p;

				p = fn.findPlug("colorR");
				p.getValue(material.diffColor[0]);
				p = fn.findPlug("colorG");
				p.getValue(material.diffColor[1]);
				p = fn.findPlug("colorB");
				p.getValue(material.diffColor[2]);
				p = fn.findPlug("colorA");
				p.getValue(material.diffColor[3]);
				p = fn.findPlug("color");

				MPlugArray connections;
				p.connectedTo(connections, true, false);

				for (int i = 0; i != connections.length(); ++i)
				{
					if (connections[i].node().apiType() == MFn::kFileTexture)
					{
						MFnDependencyNode fnDep(connections[i].node());
						MPlug filename = fnDep.findPlug("ftn");
						material.diffuseTexture = filename.asString().asChar();
						break;
					}
				}

				p = fn.findPlug("specularColorR");
				p.getValue(material.specColor[0]);
				p = fn.findPlug("specularColorG");
				p.getValue(material.specColor[1]);
				p = fn.findPlug("specularColorB");
				p.getValue(material.specColor[2]);
				p = fn.findPlug("specularColorA");
				p.getValue(material.specColor[3]);
				p = fn.findPlug("specularColor");

				p.connectedTo(connections, true, false);

				for (int i = 0; i != connections.length(); ++i)
				{
					if (connections[i].node().apiType() == MFn::kFileTexture)
					{
						MFnDependencyNode fnDep(connections[i].node());
						MPlug filename = fnDep.findPlug("ftn");
						material.specularTexture = filename.asString().asChar();
						break;
					}
				}
			}
			else if(shader.hasFn(MFn::kLambert))
			{
				MFnLambertShader fn(shader);
				MPlug p;

				p = fn.findPlug("colorR");
				p.getValue(material.diffColor[0]);
				p = fn.findPlug("colorG");
				p.getValue(material.diffColor[1]);
				p = fn.findPlug("colorB");
				p.getValue(material.diffColor[2]);
				p = fn.findPlug("colorA");
				p.getValue(material.diffColor[3]);
				p = fn.findPlug("color");

				MPlugArray connections;
				p.connectedTo(connections, true, false);

				for (int i = 0; i != connections.length(); ++i)
				{
					if (connections[i].node().apiType() == MFn::kFileTexture)
					{
						MFnDependencyNode fnDep(connections[i].node());
						MPlug filename = fnDep.findPlug("ftn");
						material.diffuseTexture = filename.asString().asChar();
						break;
					}
				}
				material.specColor[0] = -1;//Materialet har ingen specularkanal
			}
		}
	}
	return material;
}

bool ExportMesh(MFnDagNode &primaryMeshDag)
{
	//Exporterar en mesh och alla dess submeshes. Testar alla subs att deras parent är primaryMesh
	MGlobal::displayInfo(MString("Extracting Primary Mesh " + primaryMeshDag.name()));

	MFnMesh meshFN(primaryMeshDag.child(0));
	Mesh primaryMesh;
	primaryMesh.Name = primaryMeshDag.name().asChar();
	primaryMesh.geometry = ExtractGeometry(meshFN);
	primaryMesh.material = ExtractMaterial(meshFN);
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

			if (!strcmp(parentPath.fullPathName().asChar(), primaryMeshDag.fullPathName().asChar()))
				if (!dag_node.isIntermediateObject())
					{
						MGlobal::displayInfo(MString("Extracting subMesh " + dag_node.name()));

						MFnMesh subMeshFN(dag_path);
						SubMesh subMesh;
						subMesh.Name = dag_node.name().asChar();
						subMesh.geometry = ExtractGeometry(subMeshFN);

						primaryMesh.subMeshes.push_back(subMesh);
					}
		}
		subMeshes.next();
	}

	ExportFile(primaryMesh, "C:/New folder/" + primaryMesh.Name + ".bin");//Kanske ha en dialog i fönstret?
	return false;


}

void ExportFile(Mesh &mesh, std::string path)
{
	//kolla its för dokumentation
	std::ofstream outfile;
	outfile.open(path.c_str(), std::ofstream::binary);
	MainHeader mainHeader;
	mainHeader.version = 2.2;
	mainHeader.meshCount = mesh.subMeshes.size();


	outfile.write((const char*)&mainHeader, sizeof(MainHeader));

	//finding sizes of main mesh contents in header
	MeshHeader meshHeader;
	meshHeader.nameLength = mesh.Name.length()+1;
	meshHeader.numberPoints = mesh.geometry.points.size();
	meshHeader.numberNormals = mesh.geometry.normals.size();
	meshHeader.numberCoords = mesh.geometry.texCoords.size();
	meshHeader.numberFaces = mesh.geometry.faces.size();
	meshHeader.subMeshID = -1;
	meshHeader.numberPointLights = mesh.geometry.pointLights.size();
	meshHeader.numberSpotLights = mesh.geometry.spotLights.size();
	outfile.write((const char*)&meshHeader, sizeof(MeshHeader)); //writing the sizes found in the main mesh header

	//writing the main mesh contents to file according to the sizes of main mesh header
	outfile.write((const char*)mesh.Name.data(), meshHeader.nameLength);
	outfile.write((const char*)mesh.geometry.points.data(), meshHeader.numberPoints*sizeof(Point));
	outfile.write((const char*)mesh.geometry.normals.data(), meshHeader.numberNormals*sizeof(Normal));
	outfile.write((const char*)mesh.geometry.texCoords.data(), meshHeader.numberCoords*sizeof(TexCoord));

	for (int a = 0; a < meshHeader.numberFaces; a++) {
		for (int b = 0; b < 3; b++) {
			outfile.write((const char*)&mesh.geometry.faces[a].verts[b].pointID, 4);
			outfile.write((const char*)&mesh.geometry.faces[a].verts[b].normalID, 4);
			outfile.write((const char*)&mesh.geometry.faces[a].verts[b].texCoordID, 4);
		}
	}

	for (int i = 0; i < mainHeader.meshCount; i++) {

		//finding sizes of submesh contents in header
		MeshHeader meshHeader;
		meshHeader.nameLength = mesh.Name.length();
		meshHeader.numberPoints = mesh.subMeshes[i].geometry.points.size();
		meshHeader.numberNormals = mesh.subMeshes[i].geometry.normals.size();
		meshHeader.numberCoords = mesh.subMeshes[i].geometry.texCoords.size();
		meshHeader.numberFaces = mesh.subMeshes[i].geometry.faces.size();
		meshHeader.subMeshID = i;
		meshHeader.numberPointLights = mesh.subMeshes[i].geometry.pointLights.size();
		meshHeader.numberSpotLights = mesh.subMeshes[i].geometry.spotLights.size();
		outfile.write((const char*)&meshHeader, sizeof(MeshHeader));//writing the sizes found in the submesh header
	
		//writing the submesh contents to file according to the sizes of main mesh header
		outfile.write((const char*)mesh.subMeshes[i].Name.data(), meshHeader.nameLength);
		outfile.write((const char*)mesh.subMeshes[i].geometry.points.data(), meshHeader.numberPoints*sizeof(Point));
		outfile.write((const char*)mesh.subMeshes[i].geometry.normals.data(), meshHeader.numberNormals*sizeof(Normal));
		outfile.write((const char*)mesh.subMeshes[i].geometry.texCoords.data(), meshHeader.numberCoords*sizeof(TexCoord));

		for (int a = 0; a < meshHeader.numberFaces; a++) {
			for (int b = 0; b < 3; b++) {
				outfile.write((const char*)&mesh.subMeshes[i].geometry.faces[a].verts[b].pointID, 4);
				outfile.write((const char*)&mesh.subMeshes[i].geometry.faces[a].verts[b].normalID, 4);
				outfile.write((const char*)&mesh.subMeshes[i].geometry.faces[a].verts[b].texCoordID, 4);
			}
		}
	}

	MatHeader matHeader;
	matHeader.diffuseNameLength = mesh.material.diffuseTexture.length();
	matHeader.specularNameLength = mesh.material.specularTexture.length();
	outfile.write((const char*)&matHeader, sizeof(MatHeader));

	outfile.write((const char*)&mesh.material.diffColor, 16);
	outfile.write((const char*)mesh.material.diffuseTexture.data(), matHeader.diffuseNameLength);

	outfile.write((const char*)&mesh.material.specColor, 16);
	outfile.write((const char*)mesh.material.specularTexture.data(), matHeader.specularNameLength);

	outfile.close();
}