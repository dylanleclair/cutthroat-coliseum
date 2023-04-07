#include "RaceSystem.h"
#include "../entities/car/Car.h"
#include <algorithm>
#include <iostream> 

void RaceTracker::Initialize() {}

void RaceTracker::Initialize(ecs::Scene& scene)
{

  // search for all cars in the scene and add them as a contestant in the race

  for (auto entityGuid : ecs::EntitiesInScene<Car>(scene))
  {
      Car& car = scene.GetComponent<Car>(entityGuid);
      // push back incomplete ranking to m_rankings
      
      Contestant c{&car, entityGuid};
      m_contestants.push_back(c);
  }

}

void RaceTracker::Update(ecs::Scene& scene, float deltaTime) {

  // rankings should not change after the race is finished.
  if (m_raceFinished)
  {
    return;
  }

  // every tick, update the curve index of the car. check if it is near a checkpoint.
  // if it has completed all the checkpoints and is close to the initial checkpoint,
  // it's completed a lap!
  for (Contestant& car : m_contestants)
  {
    car.curveIndex = m_racepath.closestIndex(car.car->getPosition()); 

    // prevent reverse hack
    if (car.checkpoints == 0)
    {
      if (car.curveIndex > m_checkpoints[1])
      {
        car.curveIndex = 0;
      }
    }

  }

  correctIndices(m_contestants);

  for (Contestant& car : m_contestants)
  {

    // first see if the car's completed a lap
    if ((car.checkpoints == m_checkpoints.size() - 1) && abs(car.curveIndex - m_checkpoints[0]) == 0)
    {
      if (!car.isFinished)
      {
        car.lapCount++;
      }
      if (car.lapCount == MAX_LAPS+1)
      {
        car.lapCount--;
        m_raceFinished = true; // the whole race
        car.isFinished = true; // each individual racer
      }
      car.checkpoints = 0;
    }

    int next_checkpoint = (car.checkpoints + 1 == m_checkpoints.size()) ? 0 : car.checkpoints + 1; 

    if (abs(car.curveIndex - m_checkpoints[next_checkpoint]) < 3)
    {
      car.checkpoints++;
    }
  }
  computeRankings(m_contestants);

}

void RaceTracker::resetRace() {
    for (auto& contestant : m_contestants) {
        contestant.lapCount = 1;
    }

    m_raceFinished = false;
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

// if we do want to support multiplayer, we don't want to formally end the race until all
// players have finished, so we might need something like this eventually...
bool RaceTracker::isRacerFinished(Guid g)
{
  // iterate over contestants and find the matching guid
  for (auto& contestant : m_contestants)
  {
    if (contestant.guid == g && contestant.isFinished)
    {
      return true;
    }
  }
  return false;
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


void RaceTracker::computeRankings(std::vector<Contestant> contestants)
{
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
