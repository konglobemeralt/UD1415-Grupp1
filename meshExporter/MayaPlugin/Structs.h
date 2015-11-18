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

struct Geometry
{
	std::vector<Point> points;
	std::vector<Normal> normals;
	std::vector<TexCoord> texCoords;
	std::vector<Face> faces;
};

struct PointLight
{
	float pos[3], col[3], intensity, dropoff;
};

struct SpotLight
{
	float pos[3], col[3], intensity, angle, direction[3], dropoff;
};

struct SubMesh
{
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
	Geometry geometry;
};

struct Mesh
{
	std::string Name;
	int skeletonID;
	std::vector<SubMesh> subMeshes;
	std::vector<PointLight> pointLights;
	std::vector<SpotLight> spotLights;
	Geometry geometry;

	float diffColor[4], specColor[4];
	std::string diffuseTexture, specularTexture;

};
