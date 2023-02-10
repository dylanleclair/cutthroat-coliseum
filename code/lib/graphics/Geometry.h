#pragma once

//------------------------------------------------------------------------------
// This file contains simple classes for storing geomtery on the CPU and the GPU
// Later assignments will require you to expand these classes or create your own
// similar classes with the needed functionality
//------------------------------------------------------------------------------

#include "VertexArray.h"
#include "VertexBuffer.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>


// List of vertices and colour using std::vector and glm::vec3
struct CPU_Geometry {
	std::string texPath;
	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> cols;
	std::vector<glm::vec2> texs;
	std::vector<glm::vec3> norms;
};


// VAO and two VBOs for storing vertices and colours, respectively
class GPU_Geometry {

public:
	GPU_Geometry();

	// Public interface
	void bind() { vao.bind(); }

	void setVerts(const std::vector<glm::vec3>& verts);
	void setCols(const std::vector<glm::vec3>& cols);
	void setTexCoords(const std::vector<glm::vec2>& texCoords);
	void setNorms(const std::vector<glm::vec3>& norms);

private:
	// note: due to how OpenGL works, vao needs to be 
	// defined and initialized before the vertex buffers
	VertexArray vao;

	VertexBuffer vertBuffer;
	VertexBuffer colBuffer;
	VertexBuffer texCoordBuffer;
	VertexBuffer normBuffer;
};
