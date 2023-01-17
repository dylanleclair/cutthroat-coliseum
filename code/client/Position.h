#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
//TODO: Impliment rotation
/*
* This class will represent an objects position in space. It also includes
* wrappers for performing operations on its position
*/
class Position {
public:
	//sets the objects position in world space
	void setPosition(glm::vec3);
	//returns a copy of the objects position in world space
	glm::vec3 getPosision();

	//rotates the object along an axis by radian units
	//void localRotate(glm::vec3, float);
	//rotates the object around an axis through 0,0 in world space
	//it is worth noting that this changes position and rotation
	//void globalRotate(glm::vec3, float);

	//moves the object along its local axis by x units
	//void localMove(glm::vec3, float);
	//moves the object along an global axis x units
	void globalMove(glm::vec3, float);

	//computes the final transformation matrix
	glm::mat4 getTransformMatrix();
private:
	glm::mat4 position = glm::mat4(1.0f);
	//glm::quat rotation;
	glm::mat4 scale = glm::mat4(1.0f);
};