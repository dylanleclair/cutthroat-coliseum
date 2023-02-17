#include "Geometry.h"

#include <utility>


GPU_Geometry::GPU_Geometry()
	: vao()
	, vertBuffer(0, 3, GL_FLOAT)
	, normBuffer(1, 3, GL_FLOAT)
	, texCoordBuffer(2, 2, GL_FLOAT)
	, indexBuffer()
{}


void GPU_Geometry::setVerts(const std::vector<glm::vec3>& verts) {
	vertBuffer.uploadData(sizeof(glm::vec3) * verts.size(), verts.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setTexCoords(const std::vector<glm::vec2>& texCoords) {
	texCoordBuffer.uploadData(sizeof(glm::vec2) * texCoords.size(), texCoords.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setNorms(const std::vector<glm::vec3>& norms)
{
	normBuffer.uploadData(sizeof(glm::vec3) * norms.size(), norms.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setIndexBuff(const std::vector<GLuint> indices)
{
	indexBuffer.uploadData(sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
}
