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

struct Material
{
	float diffColor[4], specColor[4];
	std::string diffuseTexture, specularTexture;
};

struct PointLight
{
	float pos[3], col[3], intensity;
};

struct SpotLight
{
	float pos[3], col[3], intensity, angle, direction[3], dropoff;
};

struct Geometry
{
	std::vector<Point> points;
	std::vector<Normal> normals;
	std::vector<TexCoord> texCoords;
	std::vector<Face> faces;
	std::vector<VertexOut> vertices;
	
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
};

struct SubMesh
{
	std::string Name;
	Geometry geometry;
};

struct Mesh
{
	std::string Name;
	int skeletonID;
	std::vector<SubMesh> subMeshes;
	Geometry geometry;
	Material material;
};

//Headers-------------


struct MainHeader {
	int version, meshCount;
};

struct MeshHeader
{
	int nameLength, numberOfVertices, subMeshID, numberPointLights, numberSpotLights;
};

struct MatHeader {
	int diffuseNameLength, specularNameLength;
};