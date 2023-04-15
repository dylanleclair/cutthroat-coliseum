
// will load the assets for the game, using multithreading.
// will store them in a hashmap using the name of the asset as the key
#include <unordered_map>
#include <unordered_set>
#include "../systems/GraphicsSystem.h"

enum AssetType {
  MODEL, // renderables
  SPLINE, // splines
  COLLIDER, // collidables objects (typically want raw geom, not rendered)
};

struct Asset {
  std::string filepath;
  std::string name;
  AssetType type;

  Asset(std::string f, std::string n, AssetType t) : filepath(f), name(n), type(t) {}
};

struct AssetLoader {
  void registerAsset(std::string filename); // will set an asset to be loaded
  void loadAssets(); // will load all the assets that are registered.
  
  
  /*
  */
  void AssetLoader::loadAssetsAsync(void (*when_complete)());
  void AssetLoader::loadAssetsAsync();
  
  CPU_Geometry& getSpline(std::string name);
  CPU_Geometry& getColliderGeometry(std::string name);
  RenderModel& getRenderModel(std::string name);
  void registerAsset(std::string filename, std::string name, AssetType type)
  {
    // add to the set of registered models
    m_registered.emplace_back(filename,name,type);
  }
private:
  void loadAsset(Asset asset);
  std::unordered_map<std::string, RenderModel> m_renderables;
  std::unordered_map<std::string, CPU_Geometry> m_geoms;
  void AssetLoader::loadAsyncWorker(void (*when_complete)());

  std::vector<Asset> m_registered;
};


extern AssetLoader g_Assets; 