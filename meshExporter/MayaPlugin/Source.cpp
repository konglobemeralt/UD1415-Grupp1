#include "maya_includes.h"
#include "Structs.h"
#include "Skeleton.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <shlobj.h>

const int LEVEL_VERSION = 26;

using namespace DirectX;
using namespace std;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

//UI
void initUI();
void deleteUI();

//export
void ExportFinder(bool sl);
void ExtractLights(Mesh &mesh);
Geometry ExtractGeometry(MFnMesh &mesh, int index);
Material ExtractMaterial(MFnMesh &mesh);
bool ExportMesh(MFnDagNode &primaryMeshDag);
void ExportFile(Mesh &mesh, std::string path);
void ExportAnimation();
void GetBindPose(MObject& object, int index);
void GetAnimationLayer();
void GetAnimation();
void WriteAnimationData(std::string path);
void GenerateHitboxes();

//fileNameExtract
string getFileName(const string& string);
MString skeleton;
MString timeline;

Animation anim;

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

class exportSkeleton : public MPxCommand
{
public:
	exportSkeleton() {};
	~exportSkeleton() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("exportSkeleton Called\n");
		MGlobal::displayInfo("Third Button press!");
		ExportAnimation();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new exportSkeleton;
	}

};

class updateSkeleton : public MPxCommand
{
public:
	updateSkeleton() {};
	~updateSkeleton() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("updateSkeleton Called\n");
		MGlobal::executeCommand("textFieldGrp -q -text $skeleton;", skeleton);
		MGlobal::displayInfo(skeleton);
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new updateSkeleton;
	}

};

class updateTimeline : public MPxCommand
{
public:
	updateTimeline() {};
	~updateTimeline() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("updateTimeline Called\n");
		MGlobal::executeCommand("textFieldGrp -q -text $timeline;", timeline);
		MGlobal::displayInfo(timeline);
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new updateTimeline;
	}

};

class generateHitboxes : public MPxCommand
{
public:
	generateHitboxes() {};
	~generateHitboxes() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("generateHitboxes Called\n");
		GenerateHitboxes();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new generateHitboxes;
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
	status = plugin.registerCommand("exportSkeleton", exportSkeleton::creator);
	status = plugin.registerCommand("updateSkeleton", updateSkeleton::creator);
	status = plugin.registerCommand("updateTimeline", updateTimeline::creator);
	status = plugin.registerCommand("generateHitboxes", generateHitboxes::creator);

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
	status = plugin.deregisterCommand("exportSkeleton");
	status = plugin.deregisterCommand("updateSkeleton");
	status = plugin.deregisterCommand("updateTimeline");
	status = plugin.deregisterCommand("generateHitboxes");

	deleteUI();

	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}


void initUI()
{
	MGlobal::executeCommand("window -wh 250 105 -s true -title ""MeshExporter"" ""meshExporterUI"";");
	MGlobal::executeCommand("columnLayout -columnAttach ""left"" 5 -rowSpacing 5 -columnWidth 100;");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Generate_Hitboxes"" -command ""generateHitboxes"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Everything"" -command ""exportAll"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Selected"" -command ""exportSelected"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Export_Skeleton"" -command ""exportSkeleton"";");
	MGlobal::executeCommand("text -label ""Skeleton"";");
	MGlobal::executeCommand("$skeleton = `textFieldGrp -changeCommand ""updateSkeleton"" -text ""Unrigged"" `;");
	MGlobal::executeCommand("textFieldGrp -q -text $skeleton;", skeleton);
	MGlobal::executeCommand("text -label ""Endpoint_of_each_animation"";");
	MGlobal::executeCommand("$timeline = `textFieldGrp -changeCommand ""updateTimeline"" -text ""empty"" `;");
	MGlobal::executeCommand("textFieldGrp -q -text $timeline;", timeline);
	MGlobal::executeCommand("showWindow;");

	MGlobal::displayInfo("UI created");
}

void deleteUI()
{
	MGlobal::executeCommand("deleteUI -window ""meshExporterUI"";");
	MGlobal::displayInfo("UI deleted");
}

void ExportFinder(bool sl)//sl(selected) s?tts genom knapparnas call till ExportFinder
{

	MStringArray scene;
	if (sl)
	{
		MGlobal::executeCommand("ls -sl", scene);
	}
	else
	{
		MGlobal::executeCommand("ls", scene);
	}

	//hitta alla meshes i scenen, kolla mot objekt i listan och om de ligger p? toppniv?. Kalla ExportMesh p? alla som klarar
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

void CreateHitbox(MObject& object, unsigned int index)
{
	MFnIkJoint joint(object);
	MDoubleArray dim;
	MStringArray names;
	MGlobal::executeCommand("select " + joint.name());
	MGlobal::executeCommand("xform -q -bb", dim);
	MGlobal::executeCommand("polyCube;", names);
	MString name = names[0];
	string namestr = name.asChar();
	MGlobal::executeCommand("select -tgl " + joint.name());
	MGlobal::executeCommand("parent");
	MGlobal::executeCommand("select -r " + name + ".vtx[0] " + name + ".vtx[2] " + name + ".vtx[4] " + name + ".vtx[6];");
	MGlobal::executeCommand("move -os -x " + MString(to_string(dim[0]).c_str()) + ";");
	MGlobal::executeCommand("select -r " + name + ".vtx[1] " + name + ".vtx[3] " + name + ".vtx[5] " + name + ".vtx[7];");
	MGlobal::executeCommand("move -os -x " + MString(to_string(dim[3]).c_str()) + ";");
	MGlobal::executeCommand("select -r " + name + ".vtx[0:1] " + name + ".vtx[6:7];");
	MGlobal::executeCommand("move -os -y " + MString(to_string(dim[1]).c_str()) + ";");
	MGlobal::executeCommand("select -r " + name + ".vtx[2:5];");
	MGlobal::executeCommand("move -os -y " + MString(to_string(dim[4]).c_str()) + ";");
	MGlobal::executeCommand("select -r " + name + ".vtx[4:7];");
	MGlobal::executeCommand("move -os -z " + MString(to_string(dim[2]).c_str()) + ";");
	MGlobal::executeCommand("select -r " + name + ".vtx[0:3];");
	MGlobal::executeCommand("move -os -z " + MString(to_string(dim[5]).c_str()) + ";");
	MGlobal::executeCommand("select -r " + name);
	MGlobal::executeCommand("move -os 0");
	MGlobal::executeCommand("rotate -os 0");
	MGlobal::executeCommand("scale -os 0.15 0.15 0.15");
	MGlobal::executeCommand("FreezeTransformations");
}

void GenerateHitboxes()
{
	MItDependencyNodes itJoint(MFn::kJoint);
	unsigned int index = 0;
	for (; !itJoint.isDone(); itJoint.next()) {
		CreateHitbox(itJoint.item(), index);
		index++;
	}
}

void ExtractLights(Mesh &mesh)
{
	MItDag iter(MItDag::kBreadthFirst, MFn::kLight);
	while (!iter.isDone())
	{
		if (iter.item().hasFn(MFn::kPointLight))
		{
			MFnPointLight point = iter.item();
			PointLight pointLight;

			MFnTransform lightTransform(point.parent(0));
			MFnTransform parent(lightTransform.parent(0));
			MVector translation = parent.translation(MSpace::kObject);
			pointLight.pos[0] = (float)translation.x;
			pointLight.pos[1] = (float)translation.y;
			pointLight.pos[2] = (float)translation.z;

			pointLight.col[0] = point.color().r;
			pointLight.col[1] = point.color().g;
			pointLight.col[2] = point.color().b;
			pointLight.intensity = point.intensity();

			if (parent.object().hasFn(MFn::kJoint))
			{
				MItDag joints(MItDag::kDepthFirst, MFn::kJoint);
				int i = 0;
				while (!joints.isDone())
				{
					MFnIkJoint jointsCurr(joints.currentItem());
					if (!strcmp(jointsCurr.name().asChar(), parent.name().asChar()))
						pointLight.bone = i;
					i++;
					joints.next();
				}
				mesh.geometry.pointLights.push_back(pointLight);
			}
			else if (parent.child(0).hasFn(MFn::kMesh))
			{
				if (!strcmp(parent.name().asChar(), mesh.Name.c_str()))
					mesh.geometry.pointLights.push_back(pointLight);
			}
		}
		else if (iter.item().hasFn(MFn::kSpotLight))
		{
			MFnSpotLight spot = iter.item();
			SpotLight spotlight;

			MFnTransform lightTransform(spot.parent(0));
			MFnTransform parent(lightTransform.parent(0));
			MVector translation = parent.translation(MSpace::kObject);
			spotlight.pos[0] = (float)translation.x;
			spotlight.pos[1] = (float)translation.y;
			spotlight.pos[2] = (float)translation.z;

			spotlight.col[0] = spot.color().r;
			spotlight.col[1] = spot.color().g;
			spotlight.col[2] = spot.color().b;
			spotlight.intensity = spot.intensity();
			spotlight.angle = (float)spot.coneAngle();
			spotlight.direction[0] = spot.lightDirection().x;
			spotlight.direction[1] = spot.lightDirection().y;
			spotlight.direction[2] = spot.lightDirection().z;

			if (parent.object().hasFn(MFn::kJoint))
			{
				MItDag joints(MItDag::kDepthFirst, MFn::kJoint);
				int i = 0;
				while (!joints.isDone())
				{
					MFnIkJoint jointsCurr(joints.currentItem());
					if (!strcmp(jointsCurr.name().asChar(), parent.name().asChar()))
						spotlight.bone = i;
					i++;
					joints.next();
				}
				mesh.geometry.spotLights.push_back(spotlight);
			}
			else if (parent.object().hasFn(MFn::kMesh))
			{
				if (!strcmp(parent.name().asChar(), mesh.Name.c_str()))
					mesh.geometry.spotLights.push_back(spotlight);
			}
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

vector<WeightedVertexOut> UnpackWeightedVertices(vector<Point>* points, vector<Normal>* normals, vector<TexCoord>* UVs, vector<Face>* vertexIndices)
{
	vector<WeightedVertexOut> vertices;

	for (int i = 0; i < vertexIndices->size(); i++) {
		for (int a = 2; a > -1; a--) {
			WeightedVertexOut tempVertex;
			tempVertex.pos[0] = points->at(vertexIndices->at(i).verts[a].pointID).x;
			tempVertex.pos[1] = points->at(vertexIndices->at(i).verts[a].pointID).y;
			tempVertex.pos[2] = points->at(vertexIndices->at(i).verts[a].pointID).z;
			tempVertex.nor[0] = normals->at(vertexIndices->at(i).verts[a].normalID).x;
			tempVertex.nor[1] = normals->at(vertexIndices->at(i).verts[a].normalID).y;
			tempVertex.nor[2] = normals->at(vertexIndices->at(i).verts[a].normalID).z;
			tempVertex.uv[0] = UVs->at(vertexIndices->at(i).verts[a].texCoordID).u;
			tempVertex.uv[1] = UVs->at(vertexIndices->at(i).verts[a].texCoordID).v;

			tempVertex.influences[0] = points->at(vertexIndices->at(i).verts[a].pointID).boneIndices[0];
			tempVertex.influences[1] = points->at(vertexIndices->at(i).verts[a].pointID).boneIndices[1];
			tempVertex.influences[2] = points->at(vertexIndices->at(i).verts[a].pointID).boneIndices[2];
			tempVertex.influences[3] = points->at(vertexIndices->at(i).verts[a].pointID).boneIndices[3];

			tempVertex.weights[0] = points->at(vertexIndices->at(i).verts[a].pointID).boneWeigths[0];
			tempVertex.weights[1] = points->at(vertexIndices->at(i).verts[a].pointID).boneWeigths[1];
			tempVertex.weights[2] = points->at(vertexIndices->at(i).verts[a].pointID).boneWeigths[2];
			tempVertex.weights[3] = points->at(vertexIndices->at(i).verts[a].pointID).boneWeigths[3];

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
		MGlobal::displayInfo(name.substring(0, name.length() - 5));
		if (strcmp(skinPath.partialPathName().asChar(), name.substring(0, name.length() - 5).asChar()))
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

int TimeToAction(int time, vector<int> &actiontimes)
{
	for (int i = 0; i < actiontimes.size(); i++)
		if (time <= actiontimes[i])
			return i;
	return 0;
}

bool DivideActions()
{
	MAnimControl animControl;
	vector<string> actions;
	splitStringToVector(timeline.asChar(), actions, ",");
	vector<int> actiontimes;
	actiontimes.resize(actions.size());
	for (unsigned i = 0; i < actions.size(); i++) {
		try { actiontimes[i] = stoi(actions[i]); }
		catch (invalid_argument) {
			MGlobal::displayInfo("Export cancelled, time value failed to parse");
			return false;
		}
		catch (out_of_range) {
			MGlobal::displayInfo("Export cancelled, time value failed to parse");
			return false;
		}
		if (i != 0)
			if (actiontimes[i] <= actiontimes[i - 1]) {
				MGlobal::displayInfo("Export cancelled, time values out of order");
				return false;
			}
		if (actiontimes[i] > animControl.animationEndTime().value() || actiontimes[i] < animControl.animationStartTime().value()) {
			MGlobal::displayInfo("Export cancelled, time value outside of timeline");
			return false;
		}
	}

	anim.animLayer.resize(actiontimes.size());
	for (unsigned i = 1; i < anim.animLayer.size(); i++)
	{
		anim.animLayer[i].bones.resize(anim.animLayer[0].bones.size());
		anim.animLayer[i].nrOfFrames = actiontimes[i] - actiontimes[i-1];
	}
	for (unsigned i = 0; i < anim.animLayer[0].nrOfFrames; i++)
	{
		int action = TimeToAction(anim.animLayer[0].key[i], actiontimes);
		if (action > 0)
		{
			anim.animLayer[action].key.push_back(anim.animLayer[0].key[i]);
			anim.animLayer[action].time.push_back(anim.animLayer[0].time[i]);
			for (unsigned j = 0; j < anim.animLayer[0].bones.size(); j++)
				anim.animLayer[action].bones[j].tranform.push_back(anim.animLayer[0].bones[j].tranform[i]);
		}
	}
	//anim.animLayer[0].nrOfFrames = actiontimes[0];
	//anim.animLayer[0].key.resize(anim.animLayer[0].nrOfFrames);
	//anim.animLayer[0].time.resize(anim.animLayer[0].nrOfFrames);
	//for (unsigned i = 0; i < anim.animLayer[0].nrOfFrames; i++)
	//	anim.animLayer[0].bones[i].tranform.resize(anim.animLayer[0].nrOfFrames);
}

void ExportAnimation()
{
	// Get bind pose
	MGlobal::executeCommand("dagPose -restore -global -bindPose", false, true);
	MStatus stat;
	MItDependencyNodes itJoint(MFn::kJoint, &stat);
	MGlobal::displayInfo("LOOK HERE " + stat.errorString());
	unsigned int index = 0;
	for (; !itJoint.isDone(); itJoint.next()) {
		GetBindPose(itJoint.item(), index);
		index++;
	}
	if (index != 0)
	{
		MGlobal::executeCommand("undo", false, true);
		anim.animHeader.version = 10;
		anim.animHeader.nrOfBones = index;

		// Convert bind pose from local to world
		vector<XMFLOAT4X4> toRootTransform(anim.animHeader.nrOfBones);
		toRootTransform[0] = anim.bindPose[0].bindPose;
		for (size_t i = 1; i < anim.animHeader.nrOfBones; i++)
		{
			XMMATRIX toParent = XMLoadFloat4x4(&anim.bindPose[i].bindPose);
			XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransform[anim.bindPose[i].parent]);
			XMMATRIX toRoot = toParent * parentToRoot;
			XMStoreFloat4x4(&toRootTransform[i], toRoot);
		}
		for (size_t i = 0; i < anim.animHeader.nrOfBones; i++)
		{
			XMMATRIX temp = XMLoadFloat4x4(&toRootTransform[i]);
			temp = XMMatrixInverse(nullptr, temp);
			XMStoreFloat4x4(&anim.bindPose[i].bindPose, temp);
		}

		// Fill animation layer header
		anim.animHeader.version = 10;
		anim.animHeader.framerate = 60;
		anim.animHeader.nrOfLayers = 0;

		// Get animation layers
		GetAnimationLayer();

		// Get animation
		GetAnimation();

		if (!DivideActions()) return;
		

		char userPath[MAX_PATH];
		SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userPath);
		WriteAnimationData((string)userPath + "/Google Drive/Stort spelprojekt/ExportedModels/" + skeleton.asChar() + ".anim");
	}
}

void GetBindPose(MObject& object, int index)
{
	MFnIkJoint joint(object);
	anim.bindPose.push_back(Animation::BindPoseData());

	// Get hierarchy
	anim.boneNames.append(joint.partialPathName());
	MGlobal::displayInfo(joint.partialPathName());
	MFnIkJoint parent(joint.parent(0));
	if (index == 0) 
	{
		anim.bindPose[index].parent = -1;
	}
	else if (anim.boneNames[index - 1] == parent.partialPathName()) 
	{
		anim.bindPose[index].parent = index - 1;
	}
	else 
	{
		for (unsigned int i = 0; i < anim.boneNames.length(); i++) 
		{
			if (anim.boneNames[i] == parent.partialPathName()) 
			{
				anim.bindPose[index].parent = i;
			}
		}
	}

	// Get matrix
	double scale[3];
	double rot[4];
	MVector translation = joint.getTranslation(MSpace::kTransform);
	joint.getRotationQuaternion(rot[0], rot[1], rot[2], rot[3]);
	joint.getScale(scale);
	XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR translationV = XMVectorSet((float)translation.x, (float)translation.y, -(float)translation.z, 0.0f);
	XMVECTOR rotationV = XMVectorSet((float)rot[0], (float)rot[1], -(float)rot[2], -(float)rot[3]);
	XMVECTOR scaleV = XMVectorSet((float)scale[0], (float)scale[1], (float)scale[2], 0.0f);

	DirectX::XMStoreFloat4x4(&anim.bindPose.back().bindPose, XMMatrixAffineTransformation(scaleV, zero, rotationV, translationV));
}

void GetAnimationLayer()
{
	MAnimControl animControl;
	MTime time = animControl.playbackSpeed();
	float playBackSpeed = (float)time.value();

	MItDependencyNodes itJointAnim(MFn::kJoint);
	for (; !itJointAnim.isDone(); itJointAnim.next()) {
		MFnDependencyNode depNodeAnim(itJointAnim.item());
		MPlug animPlug = depNodeAnim.findPlug("translateX");
		MPlugArray animPlugArray;
		animPlug.connectedTo(animPlugArray, false, true);

		for (unsigned int i = 0; i < animPlugArray.length(); i++) {
			MObject layerObject = animPlugArray[i].node();
			if (layerObject.hasFn(MFn::kAnimLayer)) {
				anim.animHeader.nrOfLayers++;
				anim.animLayer.resize(anim.animHeader.nrOfLayers);
				anim.animLayer.back().nrOfFrames = 0;
				anim.animLayer.back().layerObject = layerObject;

				MFnDependencyNode layerDepNode(layerObject);
				MPlug layerPlug = layerDepNode.findPlug("foregroundWeight");
				MPlugArray layerPlugArray;
				layerPlug.connectedTo(layerPlugArray, false, true);

				bool addednrOfKeys = false;
				for (unsigned int j = 0; j < layerPlugArray.length(); j++) {
					MObject blendObject = layerPlugArray[j].node();
					if (blendObject.hasFn(MFn::kBlendNodeDoubleLinear)) {
						MFnDependencyNode blendDepNode(blendObject);
						MPlug blendPlug = blendDepNode.findPlug("inputA");	// Maybe should be inputB
						MPlugArray blendPlugArray;
						blendPlug.connectedTo(blendPlugArray, true, false);

						for (unsigned int k = 0; k < blendPlugArray.length(); k++) {
							MObject curveObject = blendPlugArray[k].node();
							if (curveObject.hasFn(MFn::kAnimCurve)) {
								MFnAnimCurve rootCurve(curveObject);

								if (rootCurve.numKeys() > 0 && addednrOfKeys == false) {
									for (unsigned int l = 0; l < rootCurve.numKeys(); l++) {
										time = rootCurve.time(l);
										anim.animLayer.back().nrOfFrames++;
										anim.animLayer.back().time.push_back(((float)time.value() / 60.0f) * playBackSpeed);
										anim.animLayer.back().key.push_back(((int)time.value()));
									}
									addednrOfKeys = true;
								}
							}
						}
					}
				}
			}
		}
	}
}

void GetAnimation()
{
	MTime time;
	MAnimControl animControl;

	for (size_t i = 0; i < anim.animHeader.nrOfLayers; i++) {
		MFnDependencyNode layerDepNode(anim.animLayer.back().layerObject);
		MGlobal::executeCommand("animLayer -solo true -edit " + layerDepNode.name(), false, true);

		MItDependencyNodes itJointKeys(MFn::kJoint);
		unsigned int boneIndex = 0;
		for (; !itJointKeys.isDone(); itJointKeys.next()) {
			anim.animLayer[i].bones.resize(anim.animLayer[i].bones.size() + 1);
			for (size_t j = 0; j < anim.animLayer[i].nrOfFrames; j++) {
				time.setValue(anim.animLayer[i].key[j]);
				animControl.setCurrentTime(time);

				// Get matrix
				MFnIkJoint joint(itJointKeys.item());
				double scale[3];
				double rot[4];
				MVector translation = joint.getTranslation(MSpace::kTransform);
				joint.getRotationQuaternion(rot[0], rot[1], rot[2], rot[3]);
				joint.getScale(scale);
				anim.animLayer[i].bones[boneIndex].tranform.push_back(Animation::FrameData());
				anim.animLayer[i].bones[boneIndex].tranform.back().translation = XMFLOAT3((float)translation.x, (float)translation.y, -(float)translation.z);
				anim.animLayer[i].bones[boneIndex].tranform.back().rotation = XMFLOAT4((float)rot[0], (float)rot[1], -(float)rot[2], -(float)rot[3]);
				anim.animLayer[i].bones[boneIndex].tranform.back().scale = XMFLOAT3((float)scale[0], (float)scale[1], (float)scale[2]);
			}
			boneIndex++;
		}
		MGlobal::executeCommand("animLayer -solo false -edit " + layerDepNode.name(), false, true);
	}

	time.setValue(1);
	animControl.setCurrentTime(time);
}

void WriteAnimationData(std::string path)
{
	std::ofstream outfile;
	// Skeleton
	outfile.open(path, std::ofstream::binary);

	if (outfile)
	{
		// Skeleton header
		outfile.write((char*)&anim.animHeader, sizeof(Animation::AnimationHeader));
		for (size_t i = 0; i < anim.animHeader.nrOfLayers; i++)
		{
			outfile.write((char*)&anim.animLayer[i].nrOfFrames, sizeof(int));
		}
		// Parents and bindposes
		outfile.write((char*)anim.bindPose.data(), sizeof(Animation::BindPoseData) * anim.animHeader.nrOfBones);

		// frames per layer
		for (size_t i = 0; i < anim.animHeader.nrOfLayers; i++)
		{
			outfile.write((char*)anim.animLayer[i].time.data(), sizeof(float) * anim.animLayer[i].nrOfFrames);

			for (size_t j = 0; j < anim.animHeader.nrOfBones; j++)
			{
				outfile.write((char*)anim.animLayer[i].bones[j].tranform.data(), sizeof(Animation::FrameData) * anim.animLayer[i].nrOfFrames);
			}
		}
		outfile.close();
	}
}

Geometry ExtractGeometry(MFnMesh &mesh, int index)
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
	geometry.index = index;
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

	if (strcmp(skeleton.asChar(), "Unrigged"))
		geometry.weightedVertices = UnpackWeightedVertices(&geometry.points, &geometry.normals, &geometry.texCoords, &geometry.faces);
	else
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

	MFnMesh* meshFN;
	MFnMesh meshFNForMaterial(primaryMeshDag.child(0));
	if (primaryMeshDag.childCount() > 1)
		meshFN = new MFnMesh(primaryMeshDag.child(primaryMeshDag.childCount()-1));
	else
		meshFN = new MFnMesh(primaryMeshDag.child(0));

	if(!strcmp(meshFN->name().asChar(), "Orig"))
		meshFN = new MFnMesh(primaryMeshDag.child(0));

	//MGlobal::displayInfo(MString("TITTA HÄR: ") + primaryMeshDag.childCount());
	MGlobal::displayInfo("TITTA HÄR: " + meshFN->name());

	Mesh primaryMesh;
	primaryMesh.Name = primaryMeshDag.name().asChar();
	primaryMesh.geometry = ExtractGeometry(*meshFN, 0);
	primaryMesh.material = ExtractMaterial(meshFNForMaterial);
	primaryMesh.skeletonID = skeleton.asChar();

	ExtractLights(primaryMesh);

	char userPath[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userPath);

	string levelVersion = "-";
	levelVersion += to_string(LEVEL_VERSION);

	ExportFile(primaryMesh, (string)userPath + "/Google Drive/Stort spelprojekt/ExportedModels/" + primaryMesh.Name + levelVersion + ".bin");//Kanske ha en dialog i fönstret?
	return false;
}

void ExportFile(Mesh &mesh, std::string path)
{
	//kolla its för dokumentation
	std::ofstream outfile;
	outfile.open(path.c_str(), std::ofstream::binary);
	MainHeader mainHeader;
	mainHeader.version = LEVEL_VERSION;
	outfile.write((const char*)&mainHeader, sizeof(MainHeader));

	int skeletonStringLength = (int)mesh.skeletonID.size();
	outfile.write((const char*)&skeletonStringLength, 4);
	outfile.write(mesh.skeletonID.data(), skeletonStringLength);

	MeshHeader meshHeader;
	if (strcmp(mesh.skeletonID.data(), "Unrigged"))
		meshHeader.numberOfVertices = (int)mesh.geometry.weightedVertices.size();
	else
		meshHeader.numberOfVertices = (int)mesh.geometry.vertices.size();
	meshHeader.numberPointLights = (int)mesh.geometry.pointLights.size();
	meshHeader.numberSpotLights = (int)mesh.geometry.spotLights.size();
	int toMesh = sizeof(MainHeader) + 4 + skeletonStringLength + 4 + sizeof(MeshHeader);
	outfile.write((char*)&toMesh, 4);

	//finding sizes of main mesh contents in header
	if (strcmp(mesh.skeletonID.data(), "Unrigged"))
		meshHeader.numberOfVertices = (int)mesh.geometry.weightedVertices.size();
	else
		meshHeader.numberOfVertices = (int)mesh.geometry.vertices.size();
	meshHeader.numberPointLights = (int)mesh.geometry.pointLights.size();
	meshHeader.numberSpotLights = (int)mesh.geometry.spotLights.size();
	outfile.write((const char*)&meshHeader, sizeof(MeshHeader)); //writing the sizes found in the main mesh header

	//writing the main mesh contents to file according to the sizes of main mesh header
	if (strcmp(mesh.skeletonID.data(), "Unrigged"))
		outfile.write((const char*)mesh.geometry.weightedVertices.data(), meshHeader.numberOfVertices * sizeof(WeightedVertexOut));
	else
		outfile.write((const char*)mesh.geometry.vertices.data(), meshHeader.numberOfVertices * sizeof(VertexOut));
	outfile.write((const char*)mesh.geometry.pointLights.data(), meshHeader.numberPointLights * sizeof(PointLight));
	outfile.write((const char*)mesh.geometry.spotLights.data(), meshHeader.numberSpotLights * sizeof(SpotLight));

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