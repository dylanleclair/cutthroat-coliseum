#include "Curve.h"

glm::vec3 Curve::nextPoint(glm::vec3 position)
{
  int closest = closestIndex(position);
  int next = nextIndex(closest);
  return m_geometry[next];
}

int  Curve::nextIndex(int index)
{
  // if the next point is outside of the array, wrap around!
  return (index + 1 == m_geometry.size()) ? 0 : index + 1; 
}

int  Curve::nextIndex(glm::vec3 pos)
{
  return nextIndex(closestIndex(pos));
}

glm::vec3  Curve::closestPoint(glm::vec3 position)
{
    return m_geometry[closestIndex(position)];
}

int  Curve::closestIndex(glm::vec3 position)
{
  // initialize with the first point on the curve.
  float distance{glm::distance(m_geometry[0], position)};
  int closestPoint{0};

  // then, iterate over all of the others!
  for (int i = 1; i < m_geometry.size(); i++)
  {
    // lookup the point
    glm::vec3 pointOnCurve = m_geometry[i];
    // compute distance b/w position and point
    float distToCurve = glm::distance(pointOnCurve, position);
    
    // if closer, replace best match.
    if (glm::distance(pointOnCurve, position) < distance)
    {
      distance = distToCurve;
      closestPoint = i;
    }
  }
  // return the result!
  return closestPoint;
}

glm::vec3  Curve::forward(glm::vec3 position)
{
  int closest = closestIndex(position);
  int next = nextIndex(closest);
  return glm::normalize(m_geometry[next] - m_geometry[closest]);
}


glm::vec3  Curve::normal(glm::vec3 position)
{
  // get the direction vector.
    glm::vec3 track_forward = forward(position);

    // fixed Y up axis
    const glm::vec3 up{0.f,1.f,0.f};

    glm::vec3 binormal = glm::cross(track_forward,up);
    glm::vec3 estimated_normal = glm::cross(track_forward, binormal);

    return glm::normalize(-estimated_normal);
}

glm::vec3& Curve::operator[](int index)
{
    return m_geometry[index];
}
  