#include "RaceSystem.h"
#include "../entities/car/Car.h"
#include "../entities/car/AICar.h"

#include <set>

void RaceTracker::Initialize() {}


void RaceTracker::Update(ecs::Scene& scene, float deltaTime) {

  // first thing we need to do is associate each guid with a position

  std::vector<Contestant> contestants;

  for (auto entityGuid : ecs::EntitiesInScene<AICar>(scene))
  {
      AICar& aicar = scene.GetComponent<AICar>(entityGuid);
      // push back incomplete Ranking to m_rankings
      
      Contestant c{entityGuid, aicar.getPosition(), -1};
      contestants.push_back(c);
  }

    for (auto entityGuid : ecs::EntitiesInScene<Car>(scene))
  {
      Car& car = scene.GetComponent<Car>(entityGuid);
      // push back incomplete Ranking to m_rankings
      
      Contestant c{entityGuid,car.getPosition(), -1};
      contestants.push_back(c);
  }

  // after getting the positions, we need to sort the rankings by distance to the finish line.
  computeRankings(contestants);
}


int RaceTracker::findClosestPointOnCurve(glm::vec3 position)
{
  float distance{std::numeric_limits<float>::max()};
  int closestPoint{0};
  for (int i = 0; i < m_racepath.size(); i++)
  {
    glm::vec3 pointOnCurve{m_racepath[i]};
    float distToCurve{glm::distance(pointOnCurve, position)};
    
    if (glm::distance(pointOnCurve, position) < distance)
    {
      distance = distToCurve;
      closestPoint = i;
    }
    return closestPoint;
  }
}

void RaceTracker::computeRankings(std::vector<Contestant> contestants)
{
  
  // iterate over every point on the curve. 
  // for each car

  auto cmp = [](Contestant a, Contestant b) {return a.curveIndex - b.curveIndex; };
  std::set<Contestant, decltype(cmp)> orderedRankings{cmp};

  for (Contestant& car : contestants)
  {
    car.curveIndex = findClosestPointOnCurve(car.worldPosition); 
    orderedRankings.insert(car);
  }

  int ranking = 1;
  // iterate over the rankings, put results in the map
  for (auto& contestant : orderedRankings)
  {
    m_rankings[contestant.carGuid] = ranking;
    ranking++;
  }

}