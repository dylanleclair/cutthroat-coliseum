#pragma once

#include "glm/glm.hpp"
#include "core/ecs.h"
#include <map>

/**
 * A RaceTracker will compute the ranking of each individual in the race! 
*/
struct RaceTracker : ecs::ISystem {
  std::vector<glm::vec3>& m_racepath;

  void Initialize();
  void Update(ecs::Scene& scene, float deltaTime);

  std::map<Guid,int> getRankings() { return m_rankings; };
private: 
  void computeRankings(std::vector<Contestant> contestants);
  int findClosestPointOnCurve(glm::vec3 position);
  std::map<Guid,int> m_rankings;
};

struct Contestant {
  Guid carGuid;
  glm::vec3 worldPosition;
  float curveIndex;
  // other data if needed I guess 
};