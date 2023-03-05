#include "components.h"
#include <iostream>
#include <algorithm>

int RenderModel::attachMesh(CPU_Geometry& _geometry)
{
	std::cout << "\tattaching mesh...\n";
	Mesh mesh = Mesh();
	//assign an ID to the mesh that is local to the model
	mesh.ID = currentMeshID;
	currentMeshID++;
	
	numberOfVerts = _geometry.verts.size();
	std::cout << "\t\tverticies: " << _geometry.verts.size() << '\n';
	mesh.geometry->setVerts(_geometry.verts);
	std::cout << "\t\tindicies: " << _geometry.indicies.size() << '\n';
	mesh.geometry->setIndexBuff(_geometry.indicies);
	//load the normals
	if (_geometry.norms.size() == numberOfVerts) {
		std::cout << "\t\tnormals: " << _geometry.norms.size() << '\n';
		mesh.geometry->setNorms(_geometry.norms);
		mesh.properties |= 0x1;
	}
	else if (_geometry.norms.size() > 0 && _geometry.norms.size() != numberOfVerts) {
		std::cout << "WARNING: Trying to attach a geometry that has a different number of normals than verticies. Assuming no normals...\n";
	}
	//load the texture coordinates
	if (_geometry.texs.size() == numberOfVerts) {
		std::cout << "\t\ttextCoords: " << _geometry.texs.size() << '\n';
		mesh.geometry->setTexCoords(_geometry.texs);
		mesh.properties |= 0x2;
	}
	else if (_geometry.texs.size() > 0 && _geometry.texs.size() != numberOfVerts) {
		std::cout << "WARNING: Trying to attach a geometry that has a different number of normals than verticies. Assuming no normals...\n";
	}
	numberOfVerts += _geometry.verts.size();
	mesh.numberOfVerticies = _geometry.verts.size();
	mesh.numberOfIndicies = _geometry.indicies.size();
	//add the mesh to the array
	meshes.push_back(mesh);
	//sorting the meshes lets us batch draws without having to rebind the same texture multiple times
	//std::sort(meshes.begin(), meshes.end(), [](const Mesh a, const Mesh b) -> bool {return a.textureIndex < b.textureIndex; });
	std::cout << "\tmesh attached with id " << mesh.ID << "\n";
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
