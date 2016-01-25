#pragma once
#include <vector>

struct Point
{
	float x, y, z;
	int boneIndices[4];
	float boneWeigths[4];
};

struct Normal
{
	float x, y, z;
};

struct skinData
{
	int points;
	int influences;
};

struct TexCoord
{
	float u, v;
};

struct Vertex
{
	int pointID, normalID, texCoordID;
};

struct Face
{
	Vertex verts[3];
};

struct VertexOut
{
	float pos[3], nor[3], uv[2];
};

struct WeightedVertexOut
{
	uint influences[4];
	float pos[3], nor[3], uv[2], weights[4];
};

struct Material
{
	float diffColor[4], specColor[4];
	std::string diffuseTexture, specularTexture;
};

struct PointLight
{
	unsigned char bone = -1;
	float pos[3], col[3], intensity;
};

struct SpotLight
{
	unsigned char bone = -1;
	float pos[3], col[3], intensity, angle, direction[3];
};

struct Geometry
{
	int index;
	std::vector<Point> points;
	std::vector<Normal> normals;
	std::vector<TexCoord> texCoords;
	std::vector<Face> faces;
	std::vector<VertexOut> vertices;
	std::vector<WeightedVertexOut> weightedVertices;
	
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
};

struct Mesh
{
	std::string Name;
	std::string skeletonID;
	Geometry geometry;
	Material material;
};

//Headers-------------


struct MainHeader {
	int version;
};

struct MeshHeader
{
	int numberOfVertices, numberPointLights, numberSpotLights;
};

struct MatHeader {
	int diffuseNameLength, specularNameLength;
};