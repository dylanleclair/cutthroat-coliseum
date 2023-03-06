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
