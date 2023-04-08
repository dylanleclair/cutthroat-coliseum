// imgui includes (essential lowkey)
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <unordered_map>
#include "../RaceSystem.h"

#include <GL/glew.h>

#include "stb_image.h"


// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);

struct UITexture {

  int width{0};
  int height{0};
  GLuint image_texture{0};
  bool isLoaded{false};

  UITexture() {}

  UITexture (const char* filename) {
    isLoaded = LoadTextureFromFile(filename,&image_texture, &width, &height);
    assert(isLoaded);
  }

  void Render() {
    ImGui::Image((void*)(intptr_t)image_texture, ImVec2(width, height));
  }

};


struct BoundingBox {
  int x,y,w,h;
};


enum MenuStatus {
  MAIN_SCREEN
};

struct LondonFog {

  LondonFog()
  {
    loadFonts();
    loadTextures();
    setStyle();
  }

  std::unordered_map<std::string,ImFont*> m_fonts;
  std::unordered_map<std::string,UITexture> m_texts;
  
  // sets the global imgui style
  void setStyle();


  MenuStatus m_status{MenuStatus::MAIN_SCREEN};


  // draws the hud for a player in their region
  void drawHUD(Guid carGuid, ecs::Scene scene, BoundingBox region, RaceTracker& raceSystem);
  void loadFonts();
  void drawMenu(int width, int height);
  void loadTextures();
};
