#include "AssetLoader.h"
#include <thread>
#include <iostream>

AssetLoader g_Assets;

void AssetLoader::loadAsset(Asset asset)
{
  std::string path = asset.filepath;
  std::string name = asset.name;
  switch (asset.type)
  {
    case MODEL: 
      m_renderables[name] = RenderModel();
      GraphicsSystem::importOBJ(m_renderables[name],path);
    case SPLINE:
      m_geoms[name] = CPU_Geometry();
      GraphicsSystem::importSplineFromOBJ(m_geoms[name],path);
  }
}


void AssetLoader::loadAssets()
{

  std::vector<std::thread> loadingThreads;
  // load the asset into the asset map
  for (auto& asset : m_registered)
  {
    std::thread t(&AssetLoader::loadAsset, this, asset); // spawn a new thread
  
    loadingThreads.push_back(move(t));
  }

  // join up all the loaders
  for (auto& thread : loadingThreads)
  {
    thread.join();
  }
  std::cout << "all assets loaded!\n";

}

CPU_Geometry& AssetLoader::getSpline(std::string name)
{
  return m_geoms[name];
}
