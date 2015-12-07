#include "MayaHeaders.h"
#include <iostream>
#include "SharedMemory.h"
#include "Skeleton.h"

#define CheckError(stat,msg)			\
    if ( MS::kSuccess != stat ) {		\
            MGlobal::displayInfo(msg);	\
    }

SharedMemory sm;

void ExtractMesh(MFnMesh& mesh);
void OutputSkinClusterTest(MObject& obj);
void ExportJoints(MObject& object);
void GetCamera(MFnCamera& camera);
void SendData();
void ExportSkeleton();
void Undo();
void Close();

vector<XMFLOAT3> vertices;
vector<int> indices;
vector<int> boneIndices;
vector<float> weights;
vector<XMFLOAT4X4> bones;

Animation anim;

struct MeshData
{
	float pos[3];
	float uv[2];
	float weights[4];
	int indices[4];
};
std::vector<MeshData> meshData;

MFloatMatrix projectionMatrix;
XMFLOAT4X4 viewMatrix;

class exportSkeleton : public MPxCommand
{
public:
	exportSkeleton() {};
	~exportSkeleton() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("exportSkeleton Called\n");
		MGlobal::displayInfo("Button press!");
		ExportSkeleton();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new exportSkeleton;
	}

};

class undo : public MPxCommand
{
public:
	undo() {};
	~undo() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("Undo Called\n");
		MGlobal::displayInfo("Button press!");
		Undo();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new undo;
	}

};

class close : public MPxCommand
{
public:
	close() {};
	~close() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("Undo Called\n");
		MGlobal::displayInfo("Button press!");
		Close();
		return MS::kSuccess;
	}

	static void* creator()
	{
		return new close;
	}

};

void initUI()
{
	MGlobal::executeCommand("window -wh 250 105 -s true -title ""SkeletonExporter"" ""SkeletonExporterUI"";");
	MGlobal::executeCommand("columnLayout -columnAttach ""left"" 5 -rowSpacing 5 -columnWidth 100;;");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""ExportSkeleton"" -command ""exportSkeleton"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Undo"" -command ""undo"";");
	MGlobal::executeCommand("button -w 200 -h 50 -label ""Close"" -command ""close"";");

	MGlobal::executeCommand("showWindow;");

	MGlobal::displayInfo("UI created");
}

void ExportSkeleton()
{
	// -------------------------------------------------------------//
	//							ANIMATION							//
	// -------------------------------------------------------------//

	// Get bind pose
	// joint -e  -oj none -ch -zso;
	MGlobal::executeCommand("dagPose -restore -global -bindPose", false, true);
	MItDependencyNodes itJoint(MFn::kJoint);
	unsigned int index = 0;
	for (; !itJoint.isDone(); itJoint.next()) {
		ExportJoints(itJoint.item());
		anim.GetBindPose(itJoint.item(), index);
		index++;
	}
	MGlobal::executeCommand("undo", false, true);
	anim.skelHead.version = 1;
	anim.skelHead.skeletonID = 0;
	anim.skelHead.nrOfBones = anim.boneData.bindPose.size();

	// Convert bind pose from local to world
	int size = anim.boneData.bindPose.size();
	vector<XMFLOAT4X4> toRootTransform(size);
	toRootTransform[0] = anim.boneData.bindPose[0];
	for (size_t i = 1; i < anim.boneData.bindPose.size(); i++)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&anim.boneData.bindPose[i]);
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransform[anim.boneData.parent[i]]);
		XMMATRIX toRoot = toParent * parentToRoot;
		XMStoreFloat4x4(&toRootTransform[i], toRoot);
	}
	for (size_t i = 0; i < anim.boneData.bindPose.size(); i++)
	{
		XMMATRIX temp = XMLoadFloat4x4(&toRootTransform[i]);
		temp = XMMatrixInverse(nullptr, temp);
		XMStoreFloat4x4(&anim.boneData.bindPose[i], temp);
		//anim.boneData.bindPose[i] = finalMatrices[i];
	}

	// Fill animation layer header
	anim.animHeader.version = 1;
	anim.animHeader.skeletonID = 0;
	anim.animHeader.framerate = 60;
	anim.animHeader.nrOfLayers = 0;

	// get animation layers
	anim.GetAnimationLayer();

	// Get animation
	anim.GetAnimation();

	// -------------------------------------------------------------//
	//								Mesh							//
	// -------------------------------------------------------------//

	// Get mesh
	MDagPath dagPath;
	MItDag itMesh(MItDag::kBreadthFirst, MFn::kMesh);
	while (!itMesh.isDone()) {
		if (itMesh.getPath(dagPath)) {
			MFnMesh mesh(itMesh.item());
			if (strstr(mesh.name().asChar(), "Orig"))
				ExtractMesh(mesh);
			else {
				MFnDependencyNode node(mesh.parent(0));
				anim.fileName = node.name().asChar();
			}
		}
		itMesh.next();
	}

	// Get weights
	MItDependencyNodes itDepNode(MFn::kSkinClusterFilter);
	for (; !itDepNode.isDone(); itDepNode.next())
	{
		MObject object = itDepNode.item();
		//OutputSkinCluster(object);
		OutputSkinClusterTest(object);
	}

	// Input bone information into vertices
	for (size_t i = 0; i < meshData.size(); i++){
		meshData[i].indices[0] = boneIndices[indices[i] * 4];
		meshData[i].indices[1] = boneIndices[(indices[i] * 4) + 1];
		meshData[i].indices[2] = boneIndices[(indices[i] * 4) + 2];
		meshData[i].indices[3] = boneIndices[(indices[i] * 4) + 3];

		meshData[i].weights[0] = weights[indices[i] * 4];
		meshData[i].weights[1] = weights[(indices[i] * 4) + 1];
		meshData[i].weights[2] = weights[(indices[i] * 4) + 2];
		meshData[i].weights[3] = weights[(indices[i] * 4) + 3];
	}

	// Write to file
	anim.WriteAnimationData();

	//SendData();
}

void Undo(){ MGlobal::executeCommand("undo", false, true); }

void Close() { 
	MGlobal::executeCommand("deleteUI -window ""SkeletonExporterUI"";");
	//MGlobal::executeCommand("unloadPlugin AnimationPluginTest"); 
}

EXPORT MStatus initializePlugin(MObject obj)
{
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "any", &res);
	if (MFAIL(res))
		CHECK_MSTATUS(res);

	if(res == MS::kSuccess)
		MGlobal::displayInfo("Maya plugin loaded!");

	myPlugin.registerCommand("exportSkeleton", exportSkeleton::creator);
	myPlugin.registerCommand("undo", undo::creator);
	myPlugin.registerCommand("close", close::creator);

	//MString memoryString;
	////sm.OpenMemory(1.0f / 256.0f)
	//memoryString = sm.OpenMemory(100.0f);
	//if (memoryString != "Shared memory open success!")
	//{
	//	MGlobal::displayInfo(memoryString);
	//	sm.CloseMemory();
	//	return MStatus::kFailure;
	//}
	//else
	//{
	//	MGlobal::displayInfo(memoryString);
	//}

	initUI();

	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	//MGlobal::displayInfo(sm.CloseMemory());
	plugin.deregisterCommand("exportSkeleton");
	plugin.deregisterCommand("undo");
	plugin.deregisterCommand("close");
	MGlobal::displayInfo("Maya plugin unloaded!");
	return MS::kSuccess;
}

void Animation::GetBindPose(MObject& object, unsigned int index)
{
	MFnIkJoint joint(object);
	anim.boneData.parent.push_back(int());

	// Get hierarchy
	boneNames.append(joint.partialPathName());
	MGlobal::displayInfo(joint.partialPathName());
	MFnIkJoint parent(joint.parent(0));
	if (index == 0){
		anim.boneData.parent.back() = -1;
	}
	else if (boneNames[index - 1] == parent.partialPathName()){
		anim.boneData.parent.back() = index - 1;
	}
	else{
		for (size_t i = 0; i < boneNames.length(); i++){
			if (boneNames[i] == parent.partialPathName()){
				anim.boneData.parent.back() = i;
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
	XMVECTOR translationV = XMVectorSet(translation.x, translation.y, -translation.z, 0.0f);
	XMVECTOR rotationV = XMVectorSet(rot[0], rot[1], -rot[2], -rot[3]);
	XMVECTOR scaleV = XMVectorSet(scale[0], scale[1], scale[2], 0.0f);

	anim.boneData.bindPose.push_back(XMFLOAT4X4());
	DirectX::XMStoreFloat4x4(&anim.boneData.bindPose.back(), XMMatrixAffineTransformation(scaleV, zero, rotationV, translationV));


	//anim.boneData.bindPose.back().translation = XMFLOAT3(translation.x, translation.y, -translation.z);
	//anim.boneData.bindPose.back().rotation = XMFLOAT4(rot[0], rot[1], -rot[2], -rot[3]);
	//anim.boneData.bindPose.back().scale = XMFLOAT3(scale[0], scale[1], -scale[2]);
}

void Animation::GetAnimationLayer()
{
	MAnimControl animControl;
	MTime time = animControl.playbackSpeed();
	float playBackSpeed = time.value();

	MItDependencyNodes itJointAnim(MFn::kJoint);
	for (; !itJointAnim.isDone(); itJointAnim.next()) {
		MFnDependencyNode depNodeAnim(itJointAnim.item());
		MPlug animPlug = depNodeAnim.findPlug("translateX");
		MPlugArray animPlugArray;
		animPlug.connectedTo(animPlugArray, false, true);

		for (size_t i = 0; i < animPlugArray.length(); i++) {
			MObject layerObject = animPlugArray[i].node();
			if (layerObject.hasFn(MFn::kAnimLayer)) {
				anim.animHeader.nrOfLayers++;
				animLayer.push_back(AnimationLayers());
				animLayer.back().layerObject = layerObject;

				MFnDependencyNode layerDepNode(layerObject);
				MPlug layerPlug = layerDepNode.findPlug("foregroundWeight");
				MPlugArray layerPlugArray;
				layerPlug.connectedTo(layerPlugArray, false, true);

				bool addednrOfKeys = false;
				for (size_t j = 0; j < layerPlugArray.length(); j++) {
					MObject blendObject = layerPlugArray[j].node();
					if (blendObject.hasFn(MFn::kBlendNodeDoubleLinear)) {
						MFnDependencyNode blendDepNode(blendObject);
						MPlug blendPlug = blendDepNode.findPlug("inputA");	// Maybe should be inputB
						MPlugArray blendPlugArray;
						blendPlug.connectedTo(blendPlugArray, true, false);

						for (size_t k = 0; k < blendPlugArray.length(); k++) {
							MObject curveObject = blendPlugArray[k].node();
							if (curveObject.hasFn(MFn::kAnimCurve)) {
								MFnAnimCurve rootCurve(curveObject);

								if (rootCurve.numKeys() > 0 && addednrOfKeys == false) {
									animLayer.back().nrOfKeys = rootCurve.numKeys();
									for (size_t l = 0; l < rootCurve.numKeys(); l++) {
										time = rootCurve.time(l);
										animLayer.back().times.push_back((time.value() / 60.0f) * playBackSpeed);
										animLayer.back().keys.push_back((time.value()));
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

void Animation::GetAnimation()
{
	MTime time;
	MAnimControl animControl;

	for (size_t i = 0; i < animHeader.nrOfLayers; i++) {
	MFnDependencyNode layerDepNode(animLayer.back().layerObject);
	MGlobal::executeCommand("animLayer -solo true -edit " + layerDepNode.name(), false, true);

		MItDependencyNodes itJointKeys(MFn::kJoint);
		unsigned int boneIndex = 0;
		for (; !itJointKeys.isDone(); itJointKeys.next()) {
			anim.animLayer[i].bones.push_back(BoneAnimation());
			for (size_t j = 0; j < animLayer[i].nrOfKeys; j++) {
				time.setValue(animLayer[i].keys[j]);
				animControl.setCurrentTime(time);

				// Get matrix
				MFnIkJoint joint(itJointKeys.item());
				double scale[3];
				double rot[4];
				MVector translation = joint.getTranslation(MSpace::kTransform);
				joint.getRotationQuaternion(rot[0], rot[1], rot[2], rot[3]);
				joint.getScale(scale);
				anim.animLayer[i].bones[boneIndex].tranform.push_back(Animation::Transform());
				anim.animLayer[i].bones[boneIndex].tranform.back().translation = XMFLOAT3(translation.x, translation.y, -translation.z);
				anim.animLayer[i].bones[boneIndex].tranform.back().rotation = XMFLOAT4(rot[0], rot[1], -rot[2], -rot[3]);
				anim.animLayer[i].bones[boneIndex].tranform.back().scale = XMFLOAT3(scale[0], scale[1], scale[2]);
			}
			boneIndex++;
		}
		MGlobal::executeCommand("animLayer -solo false -edit " + layerDepNode.name(), false, true);
	}

	time.setValue(1);
	animControl.setCurrentTime(time);
}

void Animation::WriteAnimationData()
{
	std::ofstream outfile;
	string fileDir = "C:/Users/Spelprojekt/Desktop/Spelprojekt Skeleton test/Files/";

	// Skeleton
	string skelName = anim.fileName;
	skelName = fileDir + skelName + "_Skeleton.bin";
	outfile.open(skelName, std::ofstream::binary);

	if (outfile){
		// Skeleton header
		outfile.write((char*)&anim.skelHead, sizeof(Animation::SkeletonHeader));
		// Parents and bindposes
		outfile.write((char*)anim.boneData.parent.data(), sizeof(int) * anim.skelHead.nrOfBones);
		outfile.write((char*)anim.boneData.bindPose.data(), sizeof(XMFLOAT4X4) * anim.skelHead.nrOfBones);
		outfile.close();
	}

	// Animations
	string animName = anim.fileName;
	animName = fileDir + animName + "_Animation.bin";
	outfile.open(animName, std::ofstream::binary);

	if (outfile) {
		// Header
		outfile.write((char*)&anim.animHeader, sizeof(Animation::AnimationHeader));

		// Number of keys
		for (size_t i = 0; i < anim.animHeader.nrOfLayers; i++) {
			outfile.write((char*)&anim.animLayer[i].nrOfKeys, sizeof(int) * anim.animHeader.nrOfLayers);
		}

		// Keys and transforms
		for (size_t i = 0; i < anim.animHeader.nrOfLayers; i++) {
			outfile.write((char*)anim.animLayer[i].times.data(), sizeof(float) * anim.animLayer[i].nrOfKeys);
			outfile.write((char*)anim.animLayer[i].keys.data(), sizeof(int) * anim.animLayer[i].nrOfKeys);

			for (size_t j = 0; j < anim.skelHead.nrOfBones; j++) {
				outfile.write((char*)anim.animLayer[i].bones[j].tranform.data(), sizeof(Animation::Transform) * anim.animLayer[i].nrOfKeys);
			}
		}
		outfile.close();
	}

	// Temp mesh
	string meshName = fileDir + anim.fileName + "_Mesh.bin";
	outfile.open(meshName, std::ofstream::binary);

	if (outfile){
		// Size of mesh
		unsigned int size = meshData.size();
		outfile.write((char*)&size, sizeof(int));
		// Mesh
		outfile.write((char*)meshData.data(), sizeof(MeshData) * meshData.size());
		outfile.close();
	}
}

void OutputSkinClusterTest(MObject& obj)
{
	//// Get the weights through maya's plugs...
	//MFnSkinCluster skinFn(obj);
	//// This plug is an array (one element for each vertex in your mesh
	//MPlug weightListPlug = skinFn.findPlug("weightList");
	//unsigned int vertexIndex = 0;

	//for (size_t curVert = 0; curVert < weightListPlug.numElements(); curVert++)
	//{
	//	// Get the weights for the current vertex.
	//	MPlug weightPlug = weightListPlug.elementByPhysicalIndex(curVert).child(0);

	//	// For each of the weights for this vertex
	//	unsigned int index = 0;
	//	for (size_t curWeight = 0; curWeight < weightPlug.numElements(); curWeight++)
	//	{
	//		// This weights logical index is the index to the bone that influences it
	//		boneIndices.push_back(weightPlug.elementByPhysicalIndex(curWeight).logicalIndex());

	//		// Get the weight.
	//		float weightValue;
	//		weightPlug.elementByPhysicalIndex(curWeight).getValue(weightValue);
	//		weights.push_back(weightValue);
	//		index++;
	//	}

	//	if (index < 4){
	//		for (; index < 4; index++){
	//			boneIndices.push_back(0);
	//			weights.push_back(0.0f);
	//		}
	//	}
	//	vertexIndex++;
	//}

	//unsigned int index = 0;
	//for (size_t i = 0; i < weights.size() / 4; i++)
	//{
	//	MGlobal::displayInfo(MString("Vertex: ") + i);
	//	for (size_t j = 0; j < 4; j++)
	//	{
	//		MGlobal::displayInfo(MString("Bone index: ") + boneIndices[index] + " Weight: " + weights[index]);
	//		index++;
	//	}
	//}

	// attach a skin cluster function set to
	// access the data
	MFnSkinCluster fn(obj);
	MDagPathArray infs;

	//Get influences
	int influences = fn.influenceObjects(infs);

	// loop through the geometries affected by this cluster
	int nGeoms = fn.numOutputConnections();
	for (int i = 0; i < nGeoms; ++i) {
		unsigned int index;
		index = fn.indexForOutputConnection(i);

		// get the dag path of the i'th geometry
		MDagPath skinPath;
		fn.getPathAtIndex(index, skinPath);

		// iterate through the components of this geometry
		MItGeometry gIter(skinPath);

		//Get points affected
		int points = gIter.count();

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


				MDagPath dag_path;
				MItDag dag_iter(MItDag::kBreadthFirst, MFn::kMesh);
				int currentmesh = 0, y = 0;
				while (!dag_iter.isDone())
				{
					if (dag_iter.getPath(dag_path))
					{
						MFnDagNode dag_node = dag_path.node();
						if (!dag_node.isIntermediateObject())
						{
							MFnMesh mesh(dag_path);
							if (!strcmp(skinPath.partialPathName().asChar(), mesh.partialPathName().asChar()))
								currentmesh = y;
							y++;
						}
					}

					dag_iter.next();
				}


				for (int x = 0; x < 4; x++)
				{
					boneIndices.push_back(outInfs[x]);
					weights.push_back(outWts[x] / norm);
					//mesh.vertices[gIter.index()].boneIndices[x] = outInfs[x];
					//scene_.meshes[currentmesh].points[gIter.index()].boneWeigths[x] = outWts[x] / norm;
				}
				//scene_.meshes[currentmesh].hasSkeleton = true;
			}
		}
	}
}

void ExtractMesh(MFnMesh& mesh)
{
	// Get vertices and indices
	MPointArray triPoints;
	MIntArray triIndices;
	MItMeshPolygon itPoly(mesh.object());
	MPointArray pointVertices;
	float2 uv;

	for (; !itPoly.isDone(); itPoly.next()) {
		itPoly.getTriangles(triPoints, triIndices);
		for (size_t i = 0; i < triPoints.length(); i++){
			indices.push_back(triIndices[i]);

			meshData.push_back(MeshData());
			meshData.back().pos[0] = triPoints[i].x;
			meshData.back().pos[1] = triPoints[i].y;
			meshData.back().pos[2] = -triPoints[i].z;

			itPoly.getUVAtPoint(triPoints[i], uv);
			meshData.back().uv[0] = uv[0];
			meshData.back().uv[1] = 1.0f - uv[1];
		}
	}

	mesh.getPoints(pointVertices);

	// Print previous data
	for (size_t i = 0; i < pointVertices.length(); i++) {
		vertices.push_back(XMFLOAT3(pointVertices[i].x, pointVertices[i].y, -pointVertices[i].z));
		MGlobal::displayInfo(MString("Position: ") + vertices[i].x + " " + vertices[i].y + " " + -vertices[i].z);
	}
	for (size_t i = 0; i < indices.size(); i++) {
		MGlobal::displayInfo(MString("Index: ") + indices[i]);
	}



	//MVector translation = joint.translation(MSpace::kObject);
	//MGlobal::displayInfo(MString("Translation: ") + translation.x + " " + translation.y + " " + translation.z);

	//double scale[3];
	//joint.getScale(scale);
	//MGlobal::displayInfo(MString("Scale: ") + scale[0] + " " + scale[1] + " " + scale[2]);

	//double rotation[4];
	//joint.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);
	//MGlobal::displayInfo(MString("Rotation: ") + rotation[0] + " " + rotation[1] + " " + rotation[2] + " " + rotation[3]);


}

void ExportJoints(MObject& object)
{
	//MFnWeightGeometryFilter wgf(object);
	//MFnSet setFn(wgf.deformerSet());

	//// Get connested shapes
	//MSelectionList shapes;
	//setFn.getMembers(shapes, true);


	//for (int i = 0; i < shapes.length(); ++i) {
	//	MDagPath skinpath;
	//	MObject components;
	//	MFloatArray weights;

	//	// Get the patch to current shape
	//	shapes.getDagPath(i, skinpath, components);

	//	// Get vertex weight and index
	//	wgf.getWeights(skinpath, components, weights);

	//	for (size_t k = 0; k < weights.length(); k++){
	//		MGlobal::displayInfo(MString("Weight") + weights[0]);
	//	}
	//}

	// Bone Matrix
	MFnIkJoint joint(object);
	MVector translation = joint.getTranslation(MSpace::kTransform);
	MEulerRotation eulerRot;
	joint.getRotation(eulerRot);
	double scale[3];
	double rot[4];
	joint.getScale(scale);

	XMMATRIX translationM = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
	XMMATRIX scaleM = DirectX::XMMatrixScaling(scale[0], scale[1], scale[2]);
	XMMATRIX rotationXM = DirectX::XMMatrixRotationX(eulerRot.x);
	XMMATRIX rotationYM = DirectX::XMMatrixRotationY(eulerRot.y);
	XMMATRIX rotationZM = DirectX::XMMatrixRotationZ(eulerRot.z);

	XMMATRIX finalM = scaleM * rotationXM * rotationYM * rotationZM * translationM;
	bones.push_back(XMFLOAT4X4());
	DirectX::XMStoreFloat4x4(&bones.back(), XMMatrixTranspose(finalM));

	joint.getRotationQuaternion(rot[0], rot[1], rot[2], rot[3]);
	XMVECTOR zero = XMVectorSet(0, 0, 0, 0);
	XMVECTOR translationV = XMVectorSet(translation.x, translation.y, -translation.z, 0);
	XMVECTOR rotationV = XMVectorSet(rot[0], rot[1], -rot[2], -rot[3]);
	XMVECTOR scaleV = XMVectorSet(scale[0], scale[1], -scale[2], 0);
	DirectX::XMStoreFloat4x4(&bones.back(), XMMatrixTranspose(XMMatrixAffineTransformation(scaleV, zero, rotationV, translationV)));;
}

void GetCamera(MFnCamera& camera)
{
	// Projection matrix
	projectionMatrix = camera.projectionMatrix();
	projectionMatrix[2][2] = -projectionMatrix[2][2];
	projectionMatrix[3][2] = -projectionMatrix[3][2];

	// View matrix
	MFnTransform transform(camera.parent(0));
	MMatrix transMatrix = transform.transformationMatrix().transpose().inverse();
	float pm[4][4];
	transMatrix.get(pm);
	viewMatrix = XMFLOAT4X4(
		pm[0][0], pm[0][1], pm[0][2], pm[0][3],
		pm[1][0], pm[1][1], pm[1][2], pm[1][3],
		pm[2][0], pm[2][1], pm[2][2], pm[2][3],
		pm[3][0], pm[3][1], pm[3][2], pm[3][3]);

	MVector translation = transform.getTranslation(MSpace::kTransform);
	MEulerRotation eulerRot;
	transform.getRotation(eulerRot);
	double scale[3];
	double rot[4];
	transform.getScale(scale);

	MGlobal::displayInfo(MString("Rotation X:") + eulerRot.x);
	MGlobal::displayInfo(MString("Rotation Y:") + eulerRot.y);
	MGlobal::displayInfo(MString("Rotation Z:") + eulerRot.z);

	XMMATRIX translationM = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
	XMMATRIX scaleM = DirectX::XMMatrixScaling(scale[0], scale[1], scale[2]);
	XMMATRIX rotationXM = DirectX::XMMatrixRotationX(eulerRot.x);
	XMMATRIX rotationYM = DirectX::XMMatrixRotationY(eulerRot.y);
	XMMATRIX rotationZM = DirectX::XMMatrixRotationZ(eulerRot.z);

	XMMATRIX finalM = scaleM * rotationXM * rotationYM * rotationZM * translationM;
	bones.push_back(XMFLOAT4X4());
	DirectX::XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(finalM));

	transform.getRotationQuaternion(rot[0], rot[1], rot[2], rot[3]);
	XMVECTOR zero = XMVectorSet(0, 0, 0, 0);
	XMVECTOR translationV = XMVectorSet(translation.x, translation.y, -translation.z, 0);
	XMVECTOR rotationV = XMVectorSet(rot[0], rot[1], -rot[2], -rot[3]);
	XMVECTOR scaleV = XMVectorSet(scale[0], scale[1], -scale[2], 0);
	//DirectX::XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(XMMatrixAffineTransformation(scaleV, zero, rotationV, translationV)));;
}

void SendData()
{
	unsigned int head = 0;

	//// Camera
	//memcpy((char*)sm.buffer + head, &projectionMatrix, sizeof(MFloatMatrix));
	//head += sizeof(MFloatMatrix);
	//memcpy((char*)sm.buffer + head, &viewMatrix, sizeof(XMFLOAT4X4));
	//head += sizeof(XMFLOAT4X4);

	// Vertices
	memcpy((char*)sm.buffer + head, vertices.data(), sizeof(XMFLOAT3) * vertices.size());
	head += sizeof(XMFLOAT3) * vertices.size();
	// Indices
	memcpy((char*)sm.buffer + head, indices.data(), sizeof(int) * indices.size());
	head += sizeof(int) * indices.size();
	// Bone indices
	memcpy((char*)sm.buffer + head, boneIndices.data(), sizeof(int) * boneIndices.size());
	head += sizeof(int) * boneIndices.size();
	// Bone weights
	memcpy((char*)sm.buffer + head, weights.data(), sizeof(float) * weights.size());
	head += sizeof(float) * weights.size();

	// Bone matrices
	memcpy((char*)sm.buffer + head, bones.data(), sizeof(XMFLOAT4X4));
	head += sizeof(XMFLOAT4X4);
}