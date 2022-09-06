#include <vlkx/render/Geometry.h>
using namespace Geo;

void Mesh::setTriData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices) {
	std::vector<VertexAll> Vertices = {
		{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0 },{ 0.0, 1.0 } },
		{ { 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0 },{ 0.0, 0.0 } },
		{ { -1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0 },{ 1.0, 0.0 } },
	};

	std::vector<uint32_t> Indices = {
		0, 1, 2,
	};

	vertices.clear(); indices.clear();

	vertices = Vertices;
	indices = Indices;
}

void Mesh::setQuadData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices) {

	std::vector<VertexAll> Vertices = {
		{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0 },{ 0.0, 1.0 } },
		{ { -1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0 },{ 0.0, 0.0 } },
		{ { 1.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0 },{ 1.0, 0.0 } },
		{ { 1.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0 },{ 1.0, 1.0 } }
	};

	std::vector<uint32_t> Indices = {
		0, 1, 2,
		0, 2, 3
	};

	vertices.clear(); indices.clear();
	vertices = Vertices;
	indices = Indices;
}

void Mesh::setCubeData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices) {
	std::vector<VertexAll> Vertices = {
		// Front
		{ { -1.0f, -1.0f, 1.0f },{ 0.0f, 0.0f, 1.0 },{ 0.0, 1.0 } },	// 0
		{ { -1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0 },{ 0.0, 0.0 } },		// 1
		{ { 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0 },{ 1.0, 0.0 } },		// 2
		{ { 1.0f, -1.0f, 1.0f },{ 0.0f, 0.0f, 1.0 },{ 1.0, 1.0 } },		// 3
		// Back 
		{ { 1.0, -1.0, -1.0 },{ 0.0f, 0.0f, -1.0 },{ 0.0, 1.0 } },		// 4
		{ { 1.0f, 1.0, -1.0 },{ 0.0f, 0.0f, -1.0 },{ 0.0, 0.0 } },		// 5
		{ { -1.0, 1.0, -1.0 },{ 0.0f, 0.0f, -1.0 },{ 1.0, 0.0 } },		// 6
		{ { -1.0, -1.0, -1.0 },{ 0.0f, 0.0f, -1.0 },{ 1.0, 1.0 } },		// 7
		// Left
		{ { -1.0, -1.0, -1.0 },{ -1.0f, 0.0f, 0.0 },{ 0.0, 1.0 } },		// 8
		{ { -1.0f, 1.0, -1.0 },{ -1.0f, 0.0f, 0.0 },{ 0.0, 0.0 } },		// 9
		{ { -1.0, 1.0, 1.0 },{ -1.0f, 0.0f, 0.0 },{ 1.0, 0.0 } },		// 10
		{ { -1.0, -1.0, 1.0 },{ -1.0f, 0.0f, 0.0 },{ 1.0, 1.0 } },		// 11
		// Right
		{ { 1.0, -1.0, 1.0 },{ 1.0f, 0.0f, 0.0 },{ 0.0, 1.0 } },		// 12
		{ { 1.0f, 1.0, 1.0 },{ 1.0f, 0.0f, 0.0 },{ 0.0, 0.0 } },		// 13
		{ { 1.0, 1.0, -1.0 },{ 1.0f, 0.0f, 0.0 },{ 1.0, 0.0 } },		// 14
		{ { 1.0, -1.0, -1.0 },{ 1.0f, 0.0f, 0.0 },{ 1.0, 1.0 } },		// 15
		// Top
		{ { -1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f, 0.0 },{ 0.0, 1.0 } },		// 16
		{ { -1.0f, 1.0f, -1.0f },{ 0.0f, 1.0f, 0.0 },{ 0.0, 0.0 } },	// 17
		{ { 1.0f, 1.0f, -1.0f },{ 0.0f, 1.0f, 0.0 },{ 1.0, 0.0 } },		// 18
		{ { 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f, 0.0 },{ 1.0, 1.0 } },		// 19
		// Bottom 
		{ { -1.0f, -1.0, -1.0 },{ 0.0f, -1.0f, 0.0 },{ 0.0, 1.0 } },	// 20
		{ { -1.0, -1.0, 1.0 },{ 0.0f, -1.0f, 0.0 },{ 0.0, 0.0 } },		// 21
		{ { 1.0, -1.0, 1.0 },{ 0.0f, -1.0f, 0.0 },{ 1.0, 0.0 } },		// 22
		{ { 1.0, -1.0, -1.0 },{ 0.0f, -1.0f, 0.0 },{ 1.0, 1.0 } },		// 23
	};

	std::vector<uint32_t> Indices = {
		0, 1, 2,
		2, 3, 0,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	vertices.clear(); indices.clear();
	vertices = Vertices;
	indices = Indices;
}


void Mesh::setSphereData(std::vector<VertexAll>& vertices, std::vector<uint32_t>& indices) {
	std::vector<VertexAll> Vertices;
	std::vector<uint32_t> Indices;

	float latitudeBands = 20.0f;
	float longitudeBands = 20.0f;
	float radius = 1.0f;

	for (float latNumber = 0; latNumber <= latitudeBands; latNumber++) {
		float theta = latNumber * 3.14 / latitudeBands;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);

		for (float longNumber = 0; longNumber <= longitudeBands; longNumber++) {

			float phi = longNumber * 2 * 3.147 / longitudeBands;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);

			VertexAll vs;

			vs.texture.x = (longNumber / longitudeBands); // u
			vs.texture.y = (latNumber / latitudeBands);   // v

			vs.normal.x = cosPhi * sinTheta;   // normal x
			vs.normal.y = cosTheta;            // normal y
			vs.normal.z = sinPhi * sinTheta;   // normal z

			vs.position.x = radius * vs.normal.x; // x
			vs.position.y = radius * vs.normal.y; // y
			vs.position.z = radius * vs.normal.z; // z

			Vertices.push_back(vs);
		}
	}

	for (uint32_t latNumber = 0; latNumber < latitudeBands; latNumber++) {
		for (uint32_t longNumber = 0; longNumber < longitudeBands; longNumber++) {
			uint32_t first = (latNumber * (longitudeBands + 1)) + longNumber;
			uint32_t second = first + longitudeBands + 1;

			Indices.push_back(first);
			Indices.push_back(second);
			Indices.push_back(first + 1);

			Indices.push_back(second);
			Indices.push_back(second + 1);
			Indices.push_back(first + 1);

		}
	}

	vertices.clear(); indices.clear();
	vertices = Vertices;
	indices = Indices;

}