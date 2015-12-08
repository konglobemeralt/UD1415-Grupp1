#include "maya_includes.h"
#include "Structs.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <shlobj.h>

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

//fileNameExtract
string getFileName(const string& string);
MString skeleton;

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

class updateSkeleton : public MPxCommand
{
public:
	updateSkeleton() {};
	~updateSkeleton() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("exportSelected Called\n");
		MGlobal::executeCommand("textFieldGrp -q -text $skeleton;", skeleton);
		MGlobal::displayInfo(skeleton);
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new updateSkeleton;
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
	status = plugin.registerCommand("updateSkeleton", updateSkeleton::creator);

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
	status = plugin.deregisterCommand("updateSkeleton");

	deleteUI();
	
	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}


void initUI()
{
	MGlobal::executeCommand("window -wh 250 105 -s true -title ""MeshExporter"" ""meshExporterUI"";");
	MGlobal::executeCommand("columnLayout -columnAttach ""left"" 5 -rowSpacing 5 -columnWidth 100;");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Everything"" -command ""exportAll"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Selected"" -command ""exportSelected"";");
	MGlobal::executeCommand("text -label ""Skeleton"";");
	MGlobal::executeCommand("$skeleton = `textFieldGrp -changeCommand ""updateSkeleton"" -text ""Unrigged"" `;");
	MGlobal::executeCommand("textFieldGrp -q -text $skeleton;", skeleton);
	MGlobal::executeCommand("showWindow;");
	
	MGlobal::displayInfo(skeleton);

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
				for (uint i = 0; i < scene.length(); i++)
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
	MItDag iter(MItDag::kBreadthFirst, MFn::kLight);
	while (!iter.isDone())
	{
		if (iter.item().hasFn(MFn::kPointLight))
		{
			MFnPointLight point = iter.item();
			geometry.pointLights.push_back(PointLight());

			for (size_t i = 0; i < point.parentCount(); i++)
			{
				MObject parent = point.parent((uint)i);
				if (parent.hasFn(MFn::kTransform))
				{
					MFnTransform transform(parent);
					MVector translation = transform.translation(MSpace::kObject);
					geometry.pointLights.back().pos[0] = (float)translation.x;
					geometry.pointLights.back().pos[1] = (float)translation.y;
					geometry.pointLights.back().pos[2] = (float)translation.z;
					break;
				}
			}

			geometry.pointLights.back().col[0] = point.color().r;
			geometry.pointLights.back().col[1] = point.color().g;
			geometry.pointLights.back().col[2] = point.color().b;

			geometry.pointLights.back().intensity = point.intensity();


		}
		else if (iter.item().hasFn(MFn::kSpotLight))
		{
			MFnSpotLight spot = iter.item();
			geometry.spotLights.push_back(SpotLight());


			for (size_t i = 0; i < spot.parentCount(); i++)
			{
				MObject parent = spot.parent((uint)i);
				if (parent.hasFn(MFn::kTransform))
				{
					MFnTransform transform(parent);
					MVector translation = transform.translation(MSpace::kObject);
					geometry.spotLights.back().pos[0] = (float)translation.x;
					geometry.spotLights.back().pos[1] = (float)translation.y;
					geometry.spotLights.back().pos[2] = (float)translation.z;
					break;
				}
			}

			geometry.spotLights.back().col[0] = spot.color().r;
			geometry.spotLights.back().col[1] = spot.color().g;
			geometry.spotLights.back().col[2] = spot.color().b;

			geometry.spotLights.back().intensity = spot.intensity();

			geometry.spotLights.back().angle = (float)spot.coneAngle();

			geometry.spotLights.back().direction[0] = spot.lightDirection().x;
			geometry.spotLights.back().direction[1] = spot.lightDirection().y;
			geometry.spotLights.back().direction[2] = spot.lightDirection().z;
		}
		iter.next();
	}
}

vector<VertexOut> UnpackVertices(vector<Point>* points, vector<Normal>* normals, vector<TexCoord>* UVs, vector<Face>* vertexIndices)
{
	vector<VertexOut> vertices;

	for (int i = 0; i < vertexIndices->size(); i++) {
		for (int a = 2; a > -1; a--) {
			VertexOut tempVertex;
			tempVertex.pos[0] = points->at(vertexIndices->at(i).verts[a].pointID).x;
			tempVertex.pos[1] = points->at(vertexIndices->at(i).verts[a].pointID).y;
			tempVertex.pos[2] = points->at(vertexIndices->at(i).verts[a].pointID).z;
			tempVertex.nor[0] = normals->at(vertexIndices->at(i).verts[a].normalID).x;
			tempVertex.nor[1] = normals->at(vertexIndices->at(i).verts[a].normalID).y;
			tempVertex.nor[2] = normals->at(vertexIndices->at(i).verts[a].normalID).z;
			tempVertex.uv[0] = UVs->at(vertexIndices->at(i).verts[a].texCoordID).u;
			tempVertex.uv[1] = UVs->at(vertexIndices->at(i).verts[a].texCoordID).v;
			vertices.push_back(tempVertex);
		}
	}
	return vertices;
}

void OutputSkinCluster(MObject &obj, Geometry &mesh, MString name)
{
	// attach a skin cluster function set to
	// access the data
	skinData SD;
	MFnSkinCluster fn(obj);
	MDagPathArray infs;

	//Get influences
	SD.influences = fn.influenceObjects(infs);

	// loop through the geometries affected by this cluster
	int nGeoms = fn.numOutputConnections();
	for (int i = 0; i < nGeoms; ++i) {
		unsigned int index;
		index = fn.indexForOutputConnection(i);

		// get the dag path of the i'th geometry
		MDagPath skinPath;
		fn.getPathAtIndex(index, skinPath);

		MGlobal::displayInfo(skinPath.partialPathName());
		MGlobal::displayInfo(name);
		if (strcmp(skinPath.partialPathName().asChar(), name.asChar()))
			return;

		// iterate through the components of this geometry
		MItGeometry gIter(skinPath);

		//Get points affected
		SD.points = gIter.count();

		for (; !gIter.isDone(); gIter.next()) {

			MObject comp = gIter.component();
			// Get the weights for this vertex (one per influence object)
			//
			MFloatArray wts;
			unsigned int infCount;
			fn.getWeights(skinPath, comp, wts, infCount);
			if (0 != infCount && !gIter.isDone())
			{
				int numWeights = 0;
				float outWts[40] = { 1.0f, 0 };
				int outInfs[40] = { 0 };

				// Output the weight data for this vertex
				//
				for (int j = 0; j != infCount; ++j)
				{
					// ignore weights of little effect
					if (wts[j] > 0.001f)
					{
						if (numWeights != 0)
						{
							outWts[0] -= wts[j];
							outWts[numWeights] = wts[j];
						}
						outInfs[numWeights] = j;
						++numWeights;
					}
				}
				float norm = outWts[0] + outWts[1] + outWts[2] + outWts[3];


				for (int x = 0; x < 4; x++)
				{
					mesh.points[gIter.index()].boneIndices[x] = outInfs[x];
					mesh.points[gIter.index()].boneWeigths[x] = outWts[x] / norm;
				}
			}
		}
	}
}

Geometry ExtractGeometry(MFnMesh &mesh)
{
	//// Test without polytriangulate - MAYBE NOT WORKING
	//MItMeshPolygon itPoly(mesh.object());
	//Geometry geometry;
	//Face tempface;
	//ExtractLights(mesh, geometry);

	//float2 uv;
	//MPointArray points;
	//MIntArray vertexList;
	//MFloatVectorArray normals;
	//mesh.getNormals(normals);

	//while (!itPoly.isDone())
	//{
	//	itPoly.getTriangles(points, vertexList);

	//	int lengthp = points.length();

	//	for (int i = 0; i < points.length(); i++)
	//	{
	//		// Data
	//		Point temppoints = { points[i].x, points[i].y, -points[i].z };
	//		geometry.points.push_back(temppoints);
	//		Normal tempnormals = { normals[itPoly.normalIndex(vertexList[i])].x, 
	//			normals[itPoly.normalIndex(vertexList[i])].y, 
	//			-normals[itPoly.normalIndex(vertexList[i])].z };
	//		geometry.normals.push_back(tempnormals);
	//		itPoly.getUVAtPoint(points[i], uv);
	//		TexCoord UVs = {uv[0], 1 - uv[1]};
	//		geometry.texCoords.push_back(UVs);

	//		// Indices
	//		tempface.verts[i].pointID = vertexList[i];
	//		tempface.verts[i].normalID = itPoly.normalIndex(vertexList[i]);
	//		itPoly.getUVIndex(vertexList[i], tempface.verts[i].texCoordID);
	//		geometry.faces.push_back(tempface);
	//	}
	//	itPoly.next();
	//}


	//samlar data om geometrin och sparar i ett Geometryobjekt
	Geometry geometry;
	ExtractLights(mesh, geometry);
	MSpace::Space world_space = MSpace::kTransform;

	MFloatPointArray points;
	MFloatVectorArray normals;

	//MGlobal::executeCommand(MString("deleteHistory;"));
	MGlobal::executeCommand("select " + mesh.name());
	MGlobal::executeCommand("polyTriangulate;", false, true);

	mesh.getPoints(points, world_space);
	for (uint i = 0; i < points.length(); i++)
	{
		Point temppoints = { points[i].x, points[i].y, -points[i].z };
		geometry.points.push_back(temppoints);
	}

	mesh.getNormals(normals, world_space);
	for (uint i = 0; i < normals.length(); i++)
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
	for (uint a = 0; a < Us.length(); a++)
	{
		UVs.u = Us[a];
		UVs.v = 1 - Vs[a];
		geometry.texCoords.push_back(UVs);
	}

	MStatus stat;

	MItMeshPolygon itFaces(mesh.object(), &stat);
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
	
	MGlobal::executeCommand("undo;", false, true);

	for (MItDependencyNodes it(MFn::kSkinClusterFilter); !it.isDone(); it.next())
		OutputSkinCluster(it.item(), geometry, mesh.partialPathName());

	geometry.vertices = UnpackVertices(&geometry.points, &geometry.normals, &geometry.texCoords, &geometry.faces);

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
						
						material.diffuseTexture = getFileName(material.diffuseTexture);
						
									
						
						break;
					}
				}
				material.specularTexture = "";
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
	primaryMesh.skeletonID = skeleton.asChar();

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

	char userPath[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userPath);
	ExportFile(primaryMesh, (string)userPath + "/Google Drive/Stort spelprojekt/ExportedModels/" + primaryMesh.Name + ".bin");//Kanske ha en dialog i fönstret?
	return false;
}

void ExportFile(Mesh &mesh, std::string path)
{
	//kolla its för dokumentation
	std::ofstream outfile;
	outfile.open(path.c_str(), std::ofstream::binary);
	MainHeader mainHeader;
	mainHeader.version = 24;
	mainHeader.meshCount = (int)mesh.subMeshes.size();
	outfile.write((const char*)&mainHeader, sizeof(MainHeader));

	int skeletonStringLength = mesh.skeletonID.size();
	outfile.write((const char*)&skeletonStringLength, 4);
	outfile.write(mesh.Name.data(), skeletonStringLength);

	MeshHeader meshHeader;
	meshHeader.nameLength = (int)mesh.Name.length() + 1;
	meshHeader.numberOfVertices = (int)mesh.geometry.vertices.size();
	meshHeader.subMeshID = 0;
	meshHeader.numberPointLights = (int)mesh.geometry.pointLights.size();
	meshHeader.numberSpotLights = (int)mesh.geometry.spotLights.size();
	int toMesh = sizeof(MainHeader) + sizeof(MeshHeader) + meshHeader.nameLength + (mainHeader.meshCount + 1) * 4;
	outfile.write((char*)&toMesh, 4);
	if (strcmp(mesh.skeletonID.data(), "Unrigged"))
		toMesh += meshHeader.numberOfVertices*sizeof(WeightedVertexOut);
	else
		toMesh += meshHeader.numberOfVertices*sizeof(VertexOut);
	toMesh += meshHeader.numberPointLights * sizeof(PointLight) + meshHeader.numberSpotLights * sizeof(SpotLight);

	for (int i = 0; i < mainHeader.meshCount; i++)
	{
		meshHeader.nameLength = (int)mesh.Name.length() + 1;
		meshHeader.numberOfVertices = (int)mesh.subMeshes[i].geometry.vertices.size();
		meshHeader.subMeshID = i + 1;
		meshHeader.numberPointLights = (int)mesh.subMeshes[i].geometry.pointLights.size();
		meshHeader.numberSpotLights = (int)mesh.subMeshes[i].geometry.spotLights.size();
		toMesh += sizeof(MeshHeader) + meshHeader.nameLength;
		outfile.write((char*)&toMesh, 4);
		if (strcmp(mesh.skeletonID.data(), "Unrigged"))
			toMesh += meshHeader.numberOfVertices*sizeof(WeightedVertexOut);
		else
			toMesh += meshHeader.numberOfVertices*sizeof(VertexOut);
		toMesh += meshHeader.numberPointLights * sizeof(PointLight) + meshHeader.numberSpotLights * sizeof(SpotLight);
	}

	//finding sizes of main mesh contents in header
	meshHeader.nameLength = (int)mesh.Name.length()+1;
	meshHeader.numberOfVertices = (int)mesh.geometry.vertices.size();
	meshHeader.subMeshID = 0;
	meshHeader.numberPointLights = (int)mesh.geometry.pointLights.size();
	meshHeader.numberSpotLights = (int)mesh.geometry.spotLights.size();
	outfile.write((const char*)&meshHeader, sizeof(MeshHeader)); //writing the sizes found in the main mesh header

	//writing the main mesh contents to file according to the sizes of main mesh header
	outfile.write((const char*)mesh.Name.data(), meshHeader.nameLength);
	if (strcmp(mesh.skeletonID.data(), "Unrigged"))
		outfile.write((const char*)mesh.geometry.vertices.data(), meshHeader.numberOfVertices * sizeof(WeightedVertexOut));
	else
		outfile.write((const char*)mesh.geometry.vertices.data(), meshHeader.numberOfVertices * sizeof(VertexOut));
	outfile.write((const char*)mesh.geometry.pointLights.data(), meshHeader.numberPointLights * sizeof(PointLight));
	outfile.write((const char*)mesh.geometry.spotLights.data(), meshHeader.numberSpotLights * sizeof(SpotLight));

	for (int i = 0; i < mainHeader.meshCount; i++) {

		//finding sizes of submesh contents in header
		meshHeader.nameLength = (int)mesh.Name.length()+1;
		meshHeader.numberOfVertices = (int)mesh.subMeshes[i].geometry.vertices.size();
		meshHeader.subMeshID = i + 1;
		meshHeader.numberPointLights = (int)mesh.subMeshes[i].geometry.pointLights.size();
		meshHeader.numberSpotLights = (int)mesh.subMeshes[i].geometry.spotLights.size();
		outfile.write((const char*)&meshHeader, sizeof(MeshHeader));//writing the sizes found in the submesh header
	
		//writing the submesh contents to file according to the sizes of main mesh header
		outfile.write((const char*)mesh.subMeshes[i].Name.data(), meshHeader.nameLength);
		if (strcmp(mesh.skeletonID.data(), "Unrigged"))
			outfile.write((const char*)mesh.subMeshes[i].geometry.vertices.data(), meshHeader.numberOfVertices * sizeof(WeightedVertexOut));
		else
			outfile.write((const char*)mesh.subMeshes[i].geometry.vertices.data(), meshHeader.numberOfVertices * sizeof(VertexOut));
		outfile.write((const char*)mesh.subMeshes[i].geometry.pointLights.data(), meshHeader.numberPointLights * sizeof(PointLight));
		outfile.write((const char*)mesh.subMeshes[i].geometry.spotLights.data(), meshHeader.numberSpotLights * sizeof(SpotLight));
	}

	MatHeader matHeader;
	matHeader.diffuseNameLength = (int)mesh.material.diffuseTexture.length() + 1;
	matHeader.specularNameLength = (int)mesh.material.specularTexture.length() + 1;

	if (matHeader.specularNameLength == 1)
		matHeader.specularNameLength = 0;

	if (matHeader.diffuseNameLength == 1)
		matHeader.diffuseNameLength = 0;


	std::string diffuseName = mesh.material.diffuseTexture;
	std::string specName = mesh.material.specularTexture;

	outfile.write((const char*)&matHeader, sizeof(MatHeader));

	outfile.write((const char*)&mesh.material.diffColor, 16);
	outfile.write((const char*)&mesh.material.specColor, 16);

	outfile.write((const char*)mesh.material.diffuseTexture.data(), matHeader.diffuseNameLength);
	outfile.write((const char*)mesh.material.specularTexture.data(), matHeader.specularNameLength);

	outfile.close();
}

string getFileName(const string& string)
{
	char slashChar = '/';

//#ifdef _WIN32
//	slashChar = '\\';
//#endif
//
	//size_t i = string.rfind(slashChar, string.length());

	int i = (int)string.find_last_of(slashChar);

	if (i != string.length())
	{
		//return (string.substr(i + 1));
		return(string.substr(i + 1, string.length() - 1));
	}

	return("");


}