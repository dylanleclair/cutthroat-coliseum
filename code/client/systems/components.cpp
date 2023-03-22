#include "components.h"
#include <iostream>
#include <algorithm>

int RenderModel::attachMesh(CPU_Geometry& _geometry)
{
	Mesh mesh = Mesh();
	//assign an ID to the mesh that is local to the model
	mesh.ID = currentMeshID;
	currentMeshID++;
	
	mesh.geometry->setVerts(_geometry.verts);
	mesh.geometry->setIndexBuff(_geometry.indicies);
	//load the normals
	if (_geometry.norms.size() == _geometry.verts.size()) {
		mesh.geometry->setNorms(_geometry.norms);
		mesh.properties |= 0x1;
	}

	//load the texture coordinates
	if (_geometry.texs.size() == _geometry.verts.size()) {
		mesh.geometry->setTexCoords(_geometry.texs);
		mesh.properties |= 0x2;
	}

	//update the total vertex count of the component
	numberOfVerts += _geometry.verts.size();
	mesh.numberOfVerticies = _geometry.verts.size();
	mesh.numberOfIndicies = _geometry.indicies.size();
	//add the mesh to the array
	meshes.push_back(mesh);
	//sorting the meshes lets us batch draws without having to rebind the same texture multiple times
	//std::sort(meshes.begin(), meshes.end(), [](const Mesh a, const Mesh b) -> bool {return a.textureIndex < b.textureIndex; });
	return mesh.ID;
}

bool RenderModel::attachTexture(std::string _textureName, unsigned int _meshID)
{
	_textureName = "textures/" + _textureName;
	//find the mesh with the corresponding ID
	for (Mesh& mesh : meshes) {
		if (mesh.ID == _meshID) {
			if (mesh.textureIndex != -1)
				return false;
			//determine if the model already contains the texture in its memory
			for (unsigned int i = 0; i < textures.size(); i++) {
				if (textures[i]->getPath().compare(_textureName)) {
					//if it already exists then set the meshes textureIndex to the right value
					//check that the mesh doesn't already have a texture attached
					mesh.textureIndex = i;
				}
			}
			//if no texture already exists then make a new one and attach it
			textures.push_back(new Texture(_textureName, GL_LINEAR));
			mesh.textureIndex = textures.size() - 1;
			//std::sort(meshes.begin(), meshes.end(), [](const Mesh a, const Mesh b) -> bool {return a.textureIndex < b.textureIndex; });
			for each (auto t in meshes)
				std::cout << t.textureIndex << '\n';
			return true;
		}
	}
	return false;
}

void RenderModel::setModelColor(const glm::vec3 _color)
{
	for (Mesh& mesh : meshes) {
		mesh.meshColor = _color;
	}
}

void RenderModel::isShadowed(bool isShadowed)
{
	if (isShadowed) {
		for (Mesh& mesh : meshes)
			mesh.properties |= 2;
	}
	else
	{
		for (Mesh& mesh : meshes)
			mesh.properties &= ~(2);
	}
}

RenderLine::RenderLine(const CPU_Geometry _geometry)
{
	geometry->setVerts(_geometry.verts);
	numberOfVerticies = _geometry.verts.size();
}

void RenderLine::setColor(const glm::vec3 _color)
{
	color = _color;
}

void RenderLine::setGeometry(const CPU_Geometry _geometry)
{
	geometry->setVerts(_geometry.verts);
	numberOfVerticies = _geometry.verts.size();
}

VFXBillboard::VFXBillboard(std::string _textureName)
{
	texture = new Texture(_textureName, GL_LINEAR);

}

VFXBillboard::VFXBillboard(std::string _textureName, glm::vec3 _lockingAxis)
{
	texture = new Texture(_textureName, GL_LINEAR);
	lockingAxis = _lockingAxis;
}

VFXTextureStrip::VFXTextureStrip(std::string _textureName, float _width, float _textureLength) : width(_width), textureLength(_textureLength)
{
	texture = new Texture(_textureName, GL_LINEAR);
}

VFXTextureStrip::VFXTextureStrip(std::string _textureName, const CPU_Geometry& _line, float _width, float _textureLength) : width(_width), textureLength(_textureLength)
{
	texture = new Texture(_textureName, GL_LINEAR);
	for (int i = 0; i < _line.verts.size(); i++) {
		extrude(_line.verts[i], _line.norms[i]);
	}
}

void VFXTextureStrip::extrude(glm::vec3 _position, glm::vec3 _normal)
{
	//check if the buffer is full and remove the oldest position if yes
	if (currentLength == maxLength) {
		//shift all elements back four
		for (int i = 0; i < verticies.size() - 4; i += 4) {
			verticies[i] = verticies[i + 4];
			verticies[i + 1] = verticies[i + 5];
			verticies[i + 2] = verticies[i + 6];
			verticies[i + 3] = verticies[i + 7];
		}
		//delete the last element
		verticies.pop_back();
		verticies.pop_back();
		verticies.pop_back();
		verticies.pop_back();
		currentLength--;
	}

	//update the position of the previous verticies if it can
	if (state == 1) {
		//if this is the second point we are processing then we will need to update the previous right to be accurate since it was initallty a default value
		previousRight = glm::normalize(glm::cross(_position - previousPoint, previousNormal));
		state = 2;
	}
	
	
	glm::vec3 right = glm::cross(_normal, previousPoint - _position);
	if (glm::length(right) != 0)
		right = glm::normalize(right);
	if (state != 0) {
		int i = currentLength;
		//make a quad
		verticies.push_back(previousPoint + previousRight * width);
		verticies.push_back(previousPoint - previousRight * width);
		verticies.push_back(_position + right * width);
		verticies.push_back(_position - right * width);
		float ratio = glm::length(previousPoint - _position) / textureLength;
		texCoords.push_back(glm::vec2(0, 0));
		texCoords.push_back(glm::vec2(1, 0));
		texCoords.push_back(glm::vec2(0, 1 * ratio));
		texCoords.push_back(glm::vec2(1, 1 * ratio));
		//triangle 1
		indicies.push_back(4 * i);
		indicies.push_back(4 * i + 2);
		indicies.push_back(4 * i + 3);
		//triangle 2
		indicies.push_back(4 * i);
		indicies.push_back(4 * i + 3);
		indicies.push_back(4 * i + 1);
	}

	currentLength++;

	//adjust the vertex positions to make a miter joint for the textures
	if (state == 2) {
		glm::vec3 angle = glm::normalize(previousRight + right) * width;
		//calculate the length of the 4 sides that are about to be transformed
		float a = glm::length(verticies[verticies.size() - 6] - verticies[verticies.size() - 8]);
		float b = glm::length(verticies[verticies.size() - 5] - verticies[verticies.size() - 7]);
		float c = glm::length(verticies[verticies.size() - 4] - verticies[verticies.size() - 2]);
		float d = glm::length(verticies[verticies.size() - 3] - verticies[verticies.size() - 1]);
		//transform the verticies
		verticies[verticies.size() - 3] = -angle + previousPoint;
		verticies[verticies.size() - 4] = angle + previousPoint;
		verticies[verticies.size() - 5] = -angle + previousPoint;
		verticies[verticies.size() - 6] = angle + previousPoint;
		//calculate the new side lengths
		/*
		float newa = glm::length(verticies[verticies.size() - 6] - verticies[verticies.size() - 8]);
		float newb = glm::length(verticies[verticies.size() - 5] - verticies[verticies.size() - 7]);
		float newc = glm::length(verticies[verticies.size() - 4] - verticies[verticies.size() - 2]);
		float newd = glm::length(verticies[verticies.size() - 3] - verticies[verticies.size() - 1]);
		//calculate the change in side lengths
		float ca = (a / newa) * glm::length(angle);
		float cb = (b / newb) * glm::length(angle);
		float cc = (c / newc) * glm::length(angle);
		float cd = (d / newd) * glm::length(angle);
		//adjust the UV so the texture isn't distorted by scaling by the length change
		// TODO, figure this out... But it works so I'm leaving it as is for now
		texCoords[texCoords.size() - 6] *= glm::vec2(1 - ca, 0);
		texCoords[texCoords.size() - 5] *= glm::vec2(1 - cb, 0);
		texCoords[texCoords.size() - 4] *= glm::vec2(1 - cc, 0);
		texCoords[texCoords.size() - 3] *= glm::vec2(1 - cd, 0);
		*/
		
	}

	if (state == 0)
		state = 1;

	previousPoint = _position;
	previousNormal = _normal;
	previousRight = right;

	//if the current length is 0 then add the position but write nothing to the GPU buffer
	if (currentLength <= 0) {
		GPUline->setVerts(std::vector<glm::vec3>());
		GPUline->setIndexBuff(std::vector<GLuint>());
		GPUline->setTexCoords(std::vector<glm::vec2>());
		return;
	}

	GPUline->setVerts(verticies);
	GPUline->setIndexBuff(indicies);
	GPUline->setTexCoords(texCoords);
}

void VFXTextureStrip::moveEndPoint(glm::vec3 _position)
{
}

void VFXTextureStrip::cut()
{
}

glm::vec3 VFXTextureStrip::g_previousPosition()
{
	return previousPoint;
}
