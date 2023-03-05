#pragma once
#include "../graphics/Geometry.h"
#include "../graphics/Texture.h"
#include "PxPhysicsAPI.h"

#include "utils/PxConversionUtils.h"

/*
* Transform describes the objects position in 3D space.
*/
struct TransformComponent
{
public:
	//constructors
	/*
	* This constructor will make the TransformComponent with a RigidActor already attached
	*/
	TransformComponent(physx::PxRigidActor* _actor) : actor(_actor) {};

	/*
	* This constructor will make the transform component with no defined actor attached.
	*/
	TransformComponent() {}

	/*
	* This constructor will make the transform component using glm transforms to initalize it and set its position
	*/
	TransformComponent(glm::vec3 _position, glm::quat _rotation) : position(_position)
	{
		setRotation(_rotation);
	};

	//functions to maniplate the transform
	//when getting the transform if it is attached to a physx body we need to make sure it is relative to that
	glm::vec3 getTranslation() {
		if (actor == nullptr)
			return glm::vec3(position);
		else 
			return PxtoGLM(actor->getGlobalPose().p) + glm::rotate(PxtoGLM(actor->getGlobalPose().q), position);
	}

	glm::quat getRotation() {
		glm::quat local = glm::angleAxis(rotationAngle, rotationAxis);
		if (actor == nullptr)
			return glm::quat(local);
		else
			return PxtoGLM(actor->getGlobalPose().q) * local;
	}
	glm::vec3 getScale() {
		return scale;
	}

	void setPosition(glm::vec3 _position) {
		position = _position;
	}
	void setRotation(glm::vec3 _rotationAxis, float _angle) {
		//rotationQ = glm::angleAxis(glm::radians(_angle), _rotationAxis);
		rotationAngle = _angle;
		rotationAxis = _rotationAxis;
	}
	
	void setRotation(glm::quat _rotation) {
		//rotationQ = _rotation;
		rotationAxis = glm::axis(_rotation);
		rotationAngle = glm::angle(_rotation);
	}

	void setScale(glm::vec3 _scale) {
		scale = _scale;
	}

	glm::mat4 getTransformationMatrix()
	{	
		glm::mat4 rotationM = glm::toMat4(this->getRotation());
		glm::mat4 scalingM = glm::scale(rotationM,this->getScale());
		glm::mat4 translationM = glm::translate(scalingM,this->getTranslation());

		return translationM;
	}	

private:
	friend class GraphicsSystem;
	glm::vec3 position = glm::vec3(0);
	//glm::quat rotationQ = glm::quat(1, 0, 0, 0);
	glm::vec3 rotationAxis = glm::vec3(1, 0, 0);
	float rotationAngle = 0;
	glm::vec3 scale = glm::vec3(1);
	physx::PxRigidActor* actor = nullptr;

};

struct Mesh {
	friend class GraphicsSystem;
	unsigned int numberOfVerticies = 0;
	int numberOfIndicies = 0;
	unsigned int ID = -1;
	GPU_Geometry* geometry = new GPU_Geometry;
	int textureIndex = -1;
	glm::vec3 meshColor = glm::vec3(1);
	GLuint properties = 0; //texCoords | normals
};

/*
* Render component is what is visible to the user
*/
struct RenderModel {
public:
	//constructors
	RenderModel() = default;
	//accessors
	int g_numberOfVerts() {return numberOfVerts;};
	int g_numberOfVerts(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return meshes[i].numberOfVerticies; } else { return -1; } }
	bool g_hasNormals(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return (meshes[i].properties & 1) != 0; } else { return -1; } }
	bool g_hasTextureCoords(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return (meshes[i].properties & 2) != 0; } else { return -1; } }
	std::string g_name() { return std::string(name); }

	//modifiers
	/*
	* Attaches a mesh to the render.
	* returns: -1 if a failure, local ID of the mesh otherwise
	*/
	int attachMesh(CPU_Geometry& _geometry);
	/*
	* Attaches a texture to a mesh already in the component
	* returns: true if successful, false otherwise
	*/
	bool attachTexture(std::string _texturePath, unsigned int _meshID);
	/*
	* Updates the color of all meshes in the model
	*/
	void setModelColor(const glm::vec3 _color);
//private functions
private:
	int getMeshIndex(int _meshID) {
		for (int i = 0; i < meshes.size(); i++) {
			if (meshes[i].ID == _meshID)
				return i;
		}
		return -1;
	}

//friend fields
private:
	friend class GraphicsSystem;
	//a single render component can have multiple meshes and corresponding textures attached
	std::vector<Mesh> meshes;	
	std::vector<Texture*> textures;	
	std::string name = "Dave";
//private fields
private:
	unsigned int currentMeshID = 0;
	int numberOfVerts = 0;
};

struct RenderLine {
	RenderLine() = default;
	RenderLine(const CPU_Geometry _geometry);
	void setColor(const glm::vec3 _color);
	void setGeometry(const CPU_Geometry _geometry);
private:
	friend class GraphicsSystem;
	GPU_Geometry* geometry = new GPU_Geometry();
	glm::vec3(color) = glm::vec3(1);
	GLuint numberOfVerticies = 0;
};
