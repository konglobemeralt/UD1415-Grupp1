#include "MayaHeaders.h"
#include <iostream>
#include "SharedMemory.h"

SharedMemory sm;

void ExtractMesh(MFnMesh& mesh);
void OutputSkinCluster(MObject& obj);

EXPORT MStatus initializePlugin(MObject obj)
{
	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "any", &res);
	if (MFAIL(res))
		CHECK_MSTATUS(res);

	if(res == MS::kSuccess)
		MGlobal::displayInfo("Maya plugin loaded!");

	// Open memory
	sm.OpenMemory(100);

	// Get joints
	MDagPath dagPath;
	MItDag itMesh(MItDag::kBreadthFirst, MFn::kMesh);
	while (!itMesh.isDone())
	{
		if (itMesh.getPath(dagPath))
		{
			MFnMesh mesh(itMesh.item());
			ExtractMesh(mesh);
		}
		itMesh.next();
	}

	MItDependencyNodes itDepNode(MFn::kSkinClusterFilter);
	for (; !itDepNode.isDone(); itDepNode.next())
	{
		MObject object = itDepNode.item();
		OutputSkinCluster(object);
	}

	return res;
}

EXPORT MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	MGlobal::displayInfo("Maya plugin unloaded!");
	return MS::kSuccess;
}

void OutputSkinCluster(MObject& obj)
{
	// attach a skin cluster function set to
	// access the data
	MFnSkinCluster fn(obj);

	// loop through the geometries affected by this cluster
	unsigned int index;
	index = fn.indexForOutputConnection(0);

	// get the dag path of the i'th geometry
	MDagPath skinPath;
	fn.getPathAtIndex(index, skinPath);

	// iterate through the components of this geometry
	MItGeometry gIter(skinPath);

	// print out the name of the skin cluster,
	// the vertexCount and the influenceCount
	MGlobal::displayInfo(MString("Skin: ") + skinPath.partialPathName().asChar());
	MGlobal::displayInfo(MString("pointcount: ") + gIter.count());

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
						outWts[numWeights] = wts[j];
					}
					outInfs[numWeights] = j;
					++numWeights;
				}
			}

			if (numWeights > 4){
				MGlobal::displayInfo(MString("Vikter är fel"));
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
				scene_.meshes[currentmesh].points[gIter.index()].boneIndices[x] = outInfs[x];
				scene_.meshes[currentmesh].points[gIter.index()].boneWeigths[x] = outWts[x] / norm;
			}
			scene_.meshes[currentmesh].hasSkeleton = true;
		}
	}

}

void ExtractMesh(MFnMesh& mesh)
{
	//MVector translation = joint.translation(MSpace::kObject);
	//MGlobal::displayInfo(MString("Translation: ") + translation.x + " " + translation.y + " " + translation.z);

	//double scale[3];
	//joint.getScale(scale);
	//MGlobal::displayInfo(MString("Scale: ") + scale[0] + " " + scale[1] + " " + scale[2]);

	//double rotation[4];
	//joint.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);
	//MGlobal::displayInfo(MString("Rotation: ") + rotation[0] + " " + rotation[1] + " " + rotation[2] + " " + rotation[3]);


}