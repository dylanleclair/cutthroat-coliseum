#pragma once

#include "glm/glm.hpp"
#include "core/ecs.h"
#include <map>

struct Contestant {
  Guid carGuid;
  glm::vec3 worldPosition;
  float curveIndex;
  int lapCount;
  int checkpoints;
  // other data if needed I guess 
};

/**
 * A RaceTracker will compute the ranking of each individual in the race! 
*/
struct RaceTracker : ecs::ISystem {

  const int NUM_CHECKPOINTS = 10;

  std::vector<glm::vec3>& m_racepath;
  int startIndex = 0;
  RaceTracker(std::vector<glm::vec3>& race_spline, glm::vec3 startingPoint) : m_racepath(race_spline), startIndex(findClosestPointOnCurve(startingPoint)) {
    
    int indexesBetweenCheckpoints = race_spline.size() / NUM_CHECKPOINTS;
    for (int i = 0; i < NUM_CHECKPOINTS; i++)
    {
      // calculate index of each checkpoint and add to the checkpoints array
      m_checkpoints.push_back(startIndex + indexesBetweenCheckpoints); 
    }

  }

  void Initialize();
  void Initialize(ecs::Scene& scene, float deltaTime);
  void Update(ecs::Scene& scene, float deltaTime);

  int getRanking(Guid contestantGuid);
  std::map<Guid,int>& getRankings() { return m_rankings; };
private: 
  void computeRankings(std::vector<Contestant> contestants);
  int findClosestPointOnCurve(glm::vec3 position);
  void correctIndices(std::vector<Contestant> contestants);

  // now we need to consider laps!!!

  std::map<Guid,int> m_rankings;
  std::vector<int> m_checkpoints;

};

