#pragma once
#include "graphics/Geometry.h"
#include "graphics/Texture.h"
#include "PxPhysicsAPI.h"

#include "../utils/PxConversionUtils.h"
#include <glm/gtc/random.hpp>

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

	glm::vec3 getLocalTranslation() {
		return glm::vec3(position);
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
	void attachActor(physx::PxRigidActor* _actor) {
		actor = _actor;
	}
	void detatchActor() {
		actor = nullptr;
	}

	glm::mat4 getTransformationMatrix()
	{	
		//glm::mat4 rotationM = glm::toMat4(this->getRotation());
		//glm::mat4 scalingM = glm::scale(rotationM,this->getScale());
		//glm::mat4 translationM = glm::translate(scalingM,this->getTranslation());

		return glm::translate(glm::mat4(1), getTranslation()) * toMat4(getRotation()) * glm::scale(glm::mat4(1), getScale()); 
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
	std::string name = "";
	glm::mat4 localTransformation = glm::mat4(1);
	
	//using GLuint HAS_TEX = 1;
	enum meshProperties {
		m_hasTextureCoords = 1,
		m_hasNormals = 2
	};
	GLuint properties = 0;
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
	bool g_hasNormals(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return (meshes[i].properties & 2) != 0; } else { return false; } }
	bool g_hasTextureCoords(int _meshID) { int i = getMeshIndex(_meshID); if (i != -1) { return (meshes[i].properties & 1) != 0; } else { return false; } }
	std::string g_name() { return std::string(name); }
	int g_meshIDbyName(std::string _name);
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
	bool attachTexture(std::string _texturePath, std::string _meshName);
	/*
	* Updates the color of all meshes in the model
	*/
	void setModelColor(const glm::vec3 _color);
	bool setMeshLocalTransformation(glm::mat4 _transformation, std::string _meshName);
	bool castsShadow = true;
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

struct VFXBillboard {
	enum VFXtype {
		VFX_billbaord = 0,
		VFX_textureStrip = 1
	};
	//billboard constructors
	VFXBillboard(std::string _textureName);
	VFXBillboard(std::string _textureName, glm::vec3 _lockingAxis);
private:
	friend class GraphicsSystem;
	glm::vec3 lockingAxis = glm::vec3(0);
	Texture* texture = Texture::getNoTextureTexture();
};

struct VFXTextureStrip {
	VFXTextureStrip(std::string _textureName, float _width, float textureLength = 1);
	VFXTextureStrip(std::string _textureName, const CPU_Geometry& _line, float _width, float textureLength = 1);
	void extrude(glm::vec3 _position, glm::vec3 _normal);	//extrudes a new quad from the last point (if avaiable) to _position
	void moveEndPoint(glm::vec3 _position, glm::vec3 normal); //moves the position of the last point in the texture strip
	void cut(); //cuts the strip and creates a gap between the last point and the next time extrude is called
	glm::vec3 g_previousPosition();
	
private:
	//functions
	friend class GraphicsSystem;
	GPU_Geometry* GPUline = new GPU_Geometry();
	Texture* texture = Texture::getNoTextureTexture();
	glm::vec3 position = glm::vec3(0);
	glm::vec3 normal = glm::vec3(0);
	glm::vec3 right = glm::vec3(0);
	char state = 0; //0 = we are adding the starting point, 1 = we are adding the first quad, 2 = we are adding more quads, 3 = enable joint smoothing 
	glm::vec3 previousPoint = glm::vec3(0,0,0);
	glm::vec3 previousNormal = glm::vec3(0,1,0);
	glm::vec3 previousRight = glm::vec3(0,0,0);

	float width;
	float textureLength;
	
	int currentLength = -1; //the current length of the 'spline' (-1 = uninitalized, 0 = 1 pair, >=1 number of quads
	std::vector<glm::vec2> texCoords = std::vector<glm::vec2>();
	std::vector<glm::vec3> verticies = std::vector<glm::vec3>();
	std::vector<GLuint> indicies = std::vector<GLuint>();
};

struct VFXParticleSystem {
public:
	VFXParticleSystem(std::string _textureName, size_t maxParticles = 10);
	void stepSystem(float dt, glm::mat4 transformationMatrix);
	void setMaxParticles(size_t count);

	bool active = true;
	glm::vec3 gravity = glm::vec3(0, -9.81, 0);
	float particleFrequency = 0.1; //seconds per particle spawn (lower = more frequent)
	float airResistance = 0.02;
	//velocity
	glm::vec3 initialVelocityMin = { -1, 15, -1 };
	glm::vec3 initialVelocityMax = { 1, 30, 1 };
	//position
	glm::vec3 initialPositionMin = { -1,-1,-1 };
	glm::vec3 initialPositionMax = { 1,1,1 };
	//mass
	float initialMassMin = 1;
	float initialMassMax = 1.5;
	//scale
	float initialScaleMin = 1;
	float initialScaleMax = 1;
	float scaleSpeedMin = 0;
	float scaleSpeedMax = 0;
	float scaleMaxsizeMin = 1;
	float scaleMaxsizeMax = 1;
	//rotation
	float initialRotationMin = 0;
	float initialRotationMax = 0;
	float rotationSpeedMin = 0;
	float rotationSpeedMax = 0;
	//alpha
	float initalAlphaMin = 1;
	float initalAlphaMax = 1;
	float alphaChangeMin = 0;
	float alphaChangeMax = 0;
	//lifespan
	float particleLife = 5;

private:
	friend class GraphicsSystem;
	struct Particle {
		glm::vec3 velocity = glm::vec3(0);
		float mass = 0;
		float rotationSpeed = 0; //radians/second
		float currentRotation = 0;
		float scaleSpeed = 0;
		float maxScale = 1;
		float alphaChange = 0;
		float lifespan = 0;
	};
	struct PositionData {
		glm::vec4 pos_scale = glm::vec4(0);	//x, y, z, scale
		glm::vec3 rotation = glm::vec3(1, 0, 1);	//cos, sin, alpha
	};
	std::vector<Particle> particles = std::vector<Particle>();
	std::vector<PositionData> positions = std::vector<PositionData>();
	//we need to make the vectors circular
	size_t frontIndex = 0;
	size_t backIndex = 0;

	size_t maxParticles = 0;
	float timeAccumulator = 0;
	Texture* texture = Texture::getNoTextureTexture();
	size_t particleCount = 0;
};
