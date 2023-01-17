#include "Position.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/rotate_vector.hpp>

void Position::setPosition(glm::vec3 _position)
{
	position = _position;
}

glm::vec3 Position::getPosision()
{
	return new glm::vec3(position);
}

/*
void Position::localRotate(glm::vec3 _rotationVector, float _amount)
{
	rotation = glm::rotate(rotation, _amount, _rotationVector);
}*/

//void Position::globalRotate(glm::vec3, float){}

/*
void Position::localMove(glm::vec3 _direction, float _distance)
{
	
}*/


void Position::globalMove(glm::vec3, float)
{
	//take into account the objects rotation
	position += glm::normalize(_direction) * _distance;
}

glm::mat4 Position::getTransformMatrix()
{
	return position * scale /*Rotation stuff here*/;
}