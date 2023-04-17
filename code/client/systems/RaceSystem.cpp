#include "RaceSystem.h"
#include "../entities/car/Car.h"
#include <algorithm>
#include <iostream> 


int RaceTracker::numFinishedRacers()
{
  int count{0};
  for (auto contestant : m_contestants)
  {
    if (contestant->isFinished)
    {
      count++;
    }
  }
  return count;
}

bool RaceTracker::humanRacersFinished(std::vector<Guid> humanGuids)
{
  // will check if the human racers (identified by their guid) are finished
  int countMatches = 0;

  for (auto guid : humanGuids)
  {
    ProgressTracker& pt = m_scene->GetComponent<ProgressTracker>(guid);
    if (pt.isFinished)
    {
      countMatches++;
    }
  }
  return (countMatches == humanGuids.size()) ? true : false;
}

void RaceTracker::Initialize() {}

void RaceTracker::Initialize(ecs::Scene& scene)
{

  this->m_scene = &scene;
  // search for all cars in the scene and add them as a contestant in the race

  for (auto entityGuid : ecs::EntitiesInScene<Car>(scene))
  {
      ProgressTracker& p = scene.GetComponent<ProgressTracker>(entityGuid);
      m_contestants.push_back(&p);
  }

}

void RaceTracker::Update(ecs::Scene& scene, float deltaTime) {

  // rankings should not change after the race is finished.

  // every tick, update the curve index of the car. check if it is near a checkpoint.
  // if it has completed all the checkpoints and is close to the initial checkpoint,
  // it's completed a lap!
  for (auto car : m_contestants)
  {
    Car& c = scene.GetComponent<Car>(car->guid);
    if (car->isFinished)
    {
      continue;
    }
    car->curveIndex = m_racepath.closestIndex(c.getPosition()); 

    // prevent reverse hack
    if (car->checkpoints == 0)
    {
      if (car->curveIndex > m_checkpoints[1])
      {
        car->curveIndex = 0;
      }
    }

  }

  // tbh i dont think this is even necessary anymore but we'll keep it 
  // correctIndices();

  for (auto guid : ecs::EntitiesInScene<ProgressTracker>(scene))
  {
    ProgressTracker& car = scene.GetComponent<ProgressTracker>(guid);
    Car& realCar = scene.GetComponent<Car>(guid);

    if (car.isFinished)
    {
      continue;
    }

    // first see if the car's completed a lap
    if ((car.checkpoints >= m_checkpoints.size() - 1) && abs(car.curveIndex - m_checkpoints[0]) == 0)
    {
      if (!car.isFinished)
      {
        car.lapCount++;
      }
      if (car.lapCount == MAX_LAPS+1)
      {
        car.lapCount--;
        car.isFinished = true; // each individual racer
        m_finishedRacers.push_back(guid); // mark that the racer has finished
        
      }
      car.checkpoints = 0;
    }

    int next_checkpoint = (car.checkpoints + 1 >= m_checkpoints.size()) ? 0 : car.checkpoints + 1; 

    if (abs(car.curveIndex - m_checkpoints[next_checkpoint]) < 3)
    {
      car.checkpoints++;
    }
  }
  computeRankings();

}

void RaceTracker::resetRace() {
    for (auto contestant : m_contestants) {
        // completely reset the progress tracker for each contestant
        (*contestant) = ProgressTracker{contestant->guid};
    }
    // m_raceFinished = false;
    m_finishedRacers.clear();
}

void RaceTracker::correctIndices()
{

  for (auto contestant : m_contestants)
  {
    int index = contestant->curveIndex;
    int diffy = index - startIndex;

    contestant->curveIndex = (index < startIndex) ? m_racepath.size() - diffy : index;  
  }

}

int RaceTracker::getRanking(Guid guid)
{
  return m_rankings[guid];
}




int RaceTracker::getLapCount(Guid guid)
{
  return m_scene->GetComponent<ProgressTracker>(guid).lapCount;
}

bool RaceTracker::isCarFinished(Guid guid) {
  return m_scene->GetComponent<ProgressTracker>(guid).isFinished;
}

void RaceTracker::computeRankings()
{

  m_orderedRankings.clear();
  
  auto cmp = [](ProgressTracker* a, ProgressTracker* b) {return a->raceIndex > b->raceIndex; };

  std::vector<ProgressTracker*> unfinished;

  for (auto contestant : m_contestants)
  {
    // add the lap multiple to their count
    contestant->raceIndex = contestant->curveIndex + contestant->lapCount * m_racepath.size();
    if (!contestant->isFinished)
    {
      unfinished.push_back(contestant);
    }
  }

  std::sort(m_contestants.begin(),m_contestants.end(), cmp);

  int ranking = 1;

  for (auto guid : m_finishedRacers)
  {
    m_rankings[guid] = ranking;
    m_orderedRankings.push_back(guid);
    ranking++;
  }

  // iterate over the rankings, put results in the map
  for (auto contestant : unfinished)
  {
    m_rankings[contestant->guid] = ranking;
    m_orderedRankings.push_back(contestant->guid);
    ranking++;
  }

}