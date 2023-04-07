// imgui includes (essential lowkey)
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <unordered_map>
#include "../RaceSystem.h"


struct BoundingBox {
  int x,y,w,h;
};


struct MainMenu
{
 // will render / store variables for the main menu
};

struct LondonFog {

  LondonFog()
  {
    loadFonts();
    setStyle();
  }

  std::unordered_map<std::string,ImFont*> m_fonts;
  
  // sets the global imgui style
  void setStyle();

  // draws the hud for a player in their region
  void drawHUD(Guid carGuid, ecs::Scene scene, BoundingBox region, RaceTracker& raceSystem);
  void loadFonts();

};
