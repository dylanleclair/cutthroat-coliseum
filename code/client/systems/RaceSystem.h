#pragma once

#include "glm/glm.hpp"
#include "core/ecs.h"
#include <map>
#include "../curve/Curve.h"

/**
 * The component attached to each car that will be used to track their progress
*/
struct ProgressTracker {
  int curveIndex = 0;
  int raceIndex = 0;
  int lapCount = 1;
  int checkpoints = 0;
  bool isFinished{false};
  Guid guid;
  ProgressTracker(Guid g) : guid(g) {}

  void Reset();
};

/**
 * A RaceTracker will compute the ranking of each individual in the race! 
*/
struct RaceTracker : ecs::ISystem {

  const int NUM_CHECKPOINTS = 10;
  const int MAX_LAPS = 2;

  Curve& m_racepath;
  
  int startIndex = 0;
  RaceTracker(Curve& race_spline, glm::vec3 startingPoint) : m_racepath(race_spline), startIndex(race_spline.closestIndex(startingPoint)) {
    
    int indexesBetweenCheckpoints = race_spline.size() / NUM_CHECKPOINTS;
    for (int i = 0; i < NUM_CHECKPOINTS; i++)
    {
      // calculate index of each checkpoint and add to the checkpoints array
      m_checkpoints.push_back(((indexesBetweenCheckpoints * i)) %  race_spline.size()); 
    }

  }

  void Initialize();
  void Initialize(ecs::Scene& scene);
  void Update(ecs::Scene& scene, float deltaTime);
  void resetRace();

  int getRanking(Guid contestantGuid);
  int getLapCount(Guid contestantGuid);
  
  bool getRaceStatus() { return numFinishedRacers() == m_contestants.size(); }
  bool isCarFinished(Guid guid);

bool humanRacersFinished(std::vector<Guid> humanGuids);

  std::map<Guid,int>& getRankings() { return m_rankings; };
  std::vector<Guid>& getOrderedRankings() { return m_orderedRankings; };
private: 
  void computeRankings();
  void correctIndices();

  // use to check if t
  int numFinishedRacers();
  // now we need to consider laps!!!

  std::map<Guid,int> m_rankings;
  std::vector<Guid> m_orderedRankings;
  std::vector<int> m_checkpoints;
  std::vector<ProgressTracker*> m_contestants;

  std::vector<Guid> m_finishedRacers; // always in order of first-to-last
  ecs::Scene* m_scene;

};

