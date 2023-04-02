#pragma once
#include "glm/glm.hpp"
#include <vector>

struct Curve {

  // we're going to keep track of the actual geometry, and define a few useful
  // functions that we can reuse across the project!!
  glm::vec3 forward(glm::vec3 position);
  glm::vec3 normal(glm::vec3 position);
  Curve(std::vector<glm::vec3>& points) : m_geometry(points) {}
  glm::vec3 Curve::start() {return m_geometry[0];};

  glm::vec3 nextPoint(glm::vec3 position);
  int nextIndex(int index);
  int Curve::nextIndex(glm::vec3 pos);
  glm::vec3& operator[](int);
  size_t size() { return m_geometry.size(); };
  glm::vec3 closestPoint(glm::vec3 position);
  int closestIndex(glm::vec3 position);

private: 
  std::vector<glm::vec3>& m_geometry;
};