#include "RaceSystem.h"
#include "../entities/car/Car.h"
#include "../entities/car/AICar.h"
#include <algorithm>

void RaceTracker::Initialize() {}

void RaceTracker::Initialize(ecs::Scene& scene)
{

  for (auto entityGuid : ecs::EntitiesInScene<AICar>(scene))
  {
      AICar& aicar = scene.GetComponent<AICar>(entityGuid);
      // push back incomplete Ranking to m_rankings
      
      Contestant c{&aicar, entityGuid};
      m_contestants.push_back(c);
  }

  for (auto entityGuid : ecs::EntitiesInScene<Car>(scene))
  {
      Car& car = scene.GetComponent<Car>(entityGuid);
      // push back incomplete Ranking to m_rankings
      
      Contestant c{&car, entityGuid};
      m_contestants.push_back(c);
  }

}

void RaceTracker::Update(ecs::Scene& scene, float deltaTime) {

  // every tick, update the curve index of the car. check if it is near a checkpoint.
  // if it has completed all the checkpoints and is close to the initial checkpoint,
  // it's completed a lap!
  for (Contestant& car : m_contestants)
  {
    car.curveIndex = findClosestPointOnCurve(car.car->getPosition()); 
  }

  correctIndices(m_contestants);

  for (Contestant& car : m_contestants)
  {
    // first see if the car's completed a lap
    if ((car.checkpoints == m_checkpoints.size() - 1) && abs(car.curveIndex - m_checkpoints[0]) == 0)
    {
      std::cout << "completed a lap!";
      car.lapCount++;
      car.checkpoints = 0;
    }

    int next_checkpoint = (car.checkpoints + 1 == m_checkpoints.size()) ? 0 : car.checkpoints + 1; 

    if (abs(car.curveIndex - m_checkpoints[next_checkpoint]) < 3)
    {
      car.checkpoints++;
      std::cout << "reached the next checkpoint!";
    }
  }
  computeRankings(m_contestants);

}


int RaceTracker::findClosestPointOnCurve(glm::vec3 position)
{
  float distance{std::numeric_limits<float>::max()};
  int closestPoint{0};
  for (int i = 0; i < m_racepath.size(); i++)
  {
    glm::vec3 pointOnCurve = m_racepath[i];
    float distToCurve = glm::distance(pointOnCurve, position);
    
    if (glm::distance(pointOnCurve, position) < distance)
    {
      distance = distToCurve;
      closestPoint = i;
    }
  }
    return closestPoint;
}


void RaceTracker::correctIndices(std::vector<Contestant> contestants)
{
  for (auto& contestant : contestants)
  {
    int index = contestant.curveIndex;
    int diffy = index - startIndex;


    contestant.curveIndex = (index < startIndex) ? m_racepath.size() - diffy : index;  

  }
}

int RaceTracker::getRanking(Guid contestantGuid)
{
  return m_rankings[contestantGuid];
}

int RaceTracker::getLapCount(Guid contestantGuid)
{
  for (auto& c : m_contestants)
  {
    if (c.guid == contestantGuid)
    {
      return c.lapCount;
    }
  }
  return -1;
}


// idea: use the navigationcomponent to track laps since it tracks a racer's progress anyways?
void RaceTracker::computeRankings(std::vector<Contestant> contestants)
{
  
  // iterate over every point on the curve. 
  // for each car
  auto cmp = [](Contestant a, Contestant b) {return a.curveIndex > b.curveIndex; };

  for (auto& contestant : m_contestants)
  {
    // add the lap multiple to their count
    contestant.curveIndex += contestant.lapCount * m_racepath.size();
  }

  std::sort(m_contestants.begin(),m_contestants.end(), cmp);

  int ranking = 1;
  // iterate over the rankings, put results in the map
  for (auto& contestant : m_contestants)
  {
    m_rankings[contestant.guid] = ranking;
    ranking++;
  }

}