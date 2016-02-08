#pragma once

// some definitions for the DLL to play nice with Maya
#define NT_PLUGIN
#define REQUIRE_IOSTREAM
#define EXPORT __declspec(dllexport)

#include <maya/MFnSkinCluster.h>
#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MFloatPointArray.h>
#include <maya\MDoubleArray.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MPoint.h>
#include <maya/MMatrix.h>
#include <maya/MEulerRotation.h>
#include <maya/MQuaternion.h>
#include <maya/MVector.h>
#include <maya/MItDag.h>
#include <maya/MItGeometry.h>
#include <maya/MFnCamera.h>
#include <maya/M3dView.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MImage.h>
#include <maya/MFnPointLight.h>
#include <maya/MFnSpotLight.h>
#include <maya/MSelectionList.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MItMeshVertex.h>
#include <maya/MPxPolyTrg.h>
#include <maya/MDGModifier.h>

#include <maya/MItDependencyGraph.h>
// Wrappers
#include <maya/MGlobal.h>
#include <maya/MCallbackIdArray.h>

// Animation
#include<maya/MFnIkJoint.h>
#include<maya/MFnSkinCluster.h>
#include<maya/MItGeometry.h>
#include<maya/MFnWeightGeometryFilter.h>
#include<maya/MAnimControl.h>
#include <maya/MItKeyframe.h>

// Messages
#include <maya/MMessage.h>
#include <maya/MTimerMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MEventMessage.h>
#include <maya/MPolyMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MDagMessage.h>
#include <maya/MUiMessage.h>
#include <maya/MModelMessage.h>
#include <maya/MCameraSetMessage.h>

// Libraries to link from Maya
// This can be also done in the properties setting for the project.
#pragma comment(lib,"Foundation.lib")
#pragma comment(lib,"OpenMaya.lib")
#pragma comment(lib,"OpenMayaAnim")
#pragma comment(lib,"OpenMayaUI.lib")