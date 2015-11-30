#include "maya_includes.h"
#include "overHead.h"
// basic file operations

using namespace DirectX;
using namespace std;

static const float EPS = 0.1f;
//Uncomment for txt file--------------------------------------
//std::vector<std::string> formattedLevelData;

//UI
void initUI();
void deleteUI();

//export
void exportLevelData();
void exportToFile(levelHeader, vector<mapData>);

class exportLevel : public MPxCommand
{
public:
	exportLevel() {};
	~exportLevel() {};

	virtual MStatus doIt(const MArgList&)
	{
		setResult("Export Level Called\n");
		MGlobal::displayInfo("Button press!");
		exportLevelData();
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

void exportLevelData()
{
	////For txt file, uncomment below-----------------
	////Initalizing-----------------------------------
	//formattedLevelData.clear();
	//std::string formattedOutput;

	MGlobal::displayInfo("Export level");

	MItDag itMeshes(MItDag::kDepthFirst, MFn::kMesh);

	//Level Header
	levelHeader lvlHead;
	lvlHead.version = 10;
	lvlHead.levelSIzeX = 0;
	lvlHead.levelSIzeY = 0;
	lvlHead.nrOfTileObjects = 0;

	int levelSIzeXmax = 0;
	int levelSIzeXmin = 0;
	int levelSIzeYmax = 0;
	int levelSIzeYmin = 0;

	//mapData
	std::vector<mapData> mData;

	int minX = 0, minY = 0;

	//Find all valuable data----------------------------------------
	while (!itMeshes.isDone())
	{
		MFnMesh meshTemp = itMeshes.currentItem();
		MFnTransform transformTemp = meshTemp.parent(0);

		//Name
		MString Meshname = meshTemp.name();

		MGlobal::displayInfo(MString() + "Name: " + Meshname);

		std::string tname = Meshname.asChar();

		//Translate/tile
		MFloatVector tPosition;

		tPosition = transformTemp.translation(MSpace::kTransform);

		int coordX = 0;
		if (tPosition.x < 0) {
			coordX = (int)(tPosition.x - EPS);
		}
		else {
			coordX = (int)(tPosition.x + EPS);
		}

		int coordZ = 0;
		if (tPosition.z < 0) {
			coordZ = (int)(tPosition.z - EPS);
		}
		else {
			coordZ = (int)(tPosition.z + EPS);
		}

		//Grid size
		//Maximum grid
		if (levelSIzeXmax < coordX)
		{
			levelSIzeXmax = coordX;
		}
		if (levelSIzeYmax < coordZ)
		{
			levelSIzeYmax = coordZ;
		}

		//Minimum grid
		if (levelSIzeXmin > coordX)
		{
			levelSIzeXmin = coordX;
		}
		if (levelSIzeYmin > coordZ)
		{
			levelSIzeYmin = coordZ;
		}

		//Rotation
		MEulerRotation eulerRotation;
		transformTemp.getRotation(eulerRotation);

		//Mapping the data
		mData.push_back(mapData());
		mData.back().posX = coordX;
		mData.back().posZ = coordZ;
		mData.back().rotY = eulerRotation.y;

		//Extracting enums
		if (transformTemp.hasAttribute("tileType"))
		{
			//find type of tile
			MFnEnumAttribute tileEnum = transformTemp.attribute("tileType");
			MGlobal::displayInfo("Has tiletype attr");

			MString tileType = tileEnum.fieldName(0);

			//ID/Data file names
			if (!strcmp(tileType.asChar(), "floorTile"))
			{
				MGlobal::displayInfo("tile says " + tileType);
				////Change comment between the two below----------------------------------------
				//mData.back().tileType = tileType.asChar();
				mData.back().tileType = 1;
			}
		}
		if (transformTemp.hasAttribute("walkable"))
		{
			//find if walkable or not
			MFnEnumAttribute walkEnum = transformTemp.attribute("walkable");
			MGlobal::displayInfo("has walkable attr");

			MString answer = walkEnum.fieldName(0);

			if (!strcmp(answer.asChar(), "Yes"))
			{
				MGlobal::displayInfo("walkable: yes");
				mData.back().walkable = true;
			}
			if (!strcmp(answer.asChar(), "No"))
			{
				MGlobal::displayInfo("walkable: no");
				mData.back().walkable = false;
			}
		}
		//if (transformTemp.hasAttribute("entrance"))
		//{
		//	//find if entrance or not
		//	MFnEnumAttribute entranceEnum = transformTemp.attribute("entrance");
		//	MGlobal::displayInfo("has entrance attr");

		//	MString answer = entranceEnum.fieldName(0);

		//	if (!strcmp(answer.asChar(), "Yes"))
		//	{
		//		MGlobal::displayInfo("entrance: yes");
		//		mData.back().entrance = true;
		//	}
		//	if (!strcmp(answer.asChar(), "No"))
		//	{
		//		MGlobal::displayInfo("entrance: no");
		//		mData.back().entrance = false;
		//	}
		//}
		//if (transformTemp.hasAttribute("goal"))
		//{
		//	//find if goal or not
		//	MFnEnumAttribute goalEnum = transformTemp.attribute("goal");
		//	MGlobal::displayInfo("has goal attr");
		//	MString answer = goalEnum.fieldName(0);

		//	if (!strcmp(answer.asChar(), "Yes"))
		//	{
		//		MGlobal::displayInfo("goal: yes");
		//		mData.back().goal = true;
		//	}
		//	if (!strcmp(answer.asChar(), "No"))
		//	{
		//		MGlobal::displayInfo("goal: no");
		//		mData.back().goal = false;
		//	}
		//}

		lvlHead.nrOfTileObjects++;
		itMeshes.next();
	}

	for (int i = 0; i < mData.size(); i++)
	{
		mData[i].posX -= levelSIzeXmin;
		mData[i].posZ -= levelSIzeYmin;
	}

	lvlHead.levelSIzeX = (levelSIzeXmax - levelSIzeXmin) + 1;
	lvlHead.levelSIzeY = (levelSIzeYmax - levelSIzeYmin) + 1;

	////For txt file, uncomment below (Remember to change tiletype to string in overHead.h)
	////Translating and exporting--------------------------
	//formattedOutput += "Version,";
	//formattedOutput += std::to_string(lvlHead.version);
	//formattedOutput += "\nlevelSizeX," + std::to_string(lvlHead.levelSIzeX);
	//formattedOutput += "\nlevelSizeY," + std::to_string(lvlHead.levelSIzeY);
	//formattedOutput += "\nPosX,PosY,RotY,Tiletype(ID),Walkable";
	//for (auto i : mData)
	//{
	//	formattedOutput += "\n";
	//	formattedOutput += std::to_string(i.posX) + "," + std::to_string(i.posZ);
	//	formattedOutput += ",";
	//	formattedOutput += std::to_string(i.rotY);

	//	//Change between the two i.tileType--------------
	//	formattedOutput += "," + i.tileType;
	//	//formattedOutput += "," + std::to_string(i.tileType);
	//	formattedOutput += "," + std::to_string(i.walkable);
	//	//formattedOutput += "," + std::to_string(i.entrance);
	//	//formattedOutput += "," + std::to_string(i.goal);
	//}

	//formattedLevelData.push_back(formattedOutput);
	exportToFile(lvlHead, mData);
}

void exportToFile(levelHeader lvlHead, vector<mapData> mData)
{
	string outputPath;
	string levelName = "level1.lvl";

	char userPath[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, userPath);
	outputPath = (string)userPath + "\\Google Drive\\Stort spelprojekt\\ExportedLevels";
	ofstream outputFile;
	_mkdir(outputPath.c_str());
	outputPath += "\\" + levelName;

	//To change to txt file--------------------------------
	//Comment/remove binary
	outputFile.open(outputPath, ios::out | ios::binary);

	//Comment below
	outputFile.write((const char*)&lvlHead, sizeof(lvlHead));

	for (auto md : mData)
	{
		outputFile.write((const char*)&md, sizeof(mapData));
	}

	//Uncomment this section and comment above--------------
	//for (std::string formattedOutput : formattedLevelData)
	//{
	//	outputFile << formattedOutput;
	//}

	outputFile.close();
}

