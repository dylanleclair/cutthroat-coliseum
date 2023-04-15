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
    // will just render full size
    ImGui::Image((void*)(intptr_t)image_texture, ImVec2(width, height));
  }

  void Render(float w, float h) {
    ImGui::Image((void*)(intptr_t)image_texture, ImVec2(w, h));
  }

    void Render(ImVec2 size) {
    ImGui::Image((void*)(intptr_t)image_texture, size);
  }

};

enum Corner {
  TOP_LEFT,
  TOP_RIGHT,
  BOTTOM_LEFT,
  BOTTOM_RIGHT
};
struct BoundingBox {
  int x,y,w,h;

  ImVec2 getRelativeCenter(ImVec2 elementSize)
  {
    // compute center of the bounding box.
    float centerX = static_cast<float>(x) + (static_cast<float>(w) / 2.f);
    float centerY = static_cast<float>(y) + (static_cast<float>(h) / 2.f);
    // then subtract half of the elements size's width/height
    centerX -= (elementSize.x) / 2.f;
    centerY -= (elementSize.y) / 2.f;
    
    // return the computer center coords
    return ImVec2{centerX,centerY};
  }

  ImVec2 getCorner(Corner c)
  {

    float xf = static_cast<float>(x);
    float yf = static_cast<float>(y);
    float wf = static_cast<float>(w);
    float hf = static_cast<float>(h);

    switch (c)
    {
      case (TOP_LEFT):
        return ImVec2(xf, yf);
      case (TOP_RIGHT):
        return ImVec2(xf + wf, yf);
      case (BOTTOM_LEFT):
        return ImVec2(xf, yf + hf);
      case (BOTTOM_RIGHT):
        return ImVec2(xf + wf, yf + hf);
      default:
        return ImVec2();
    }
  }

};


enum MenuStatus {
  MAIN_SCREEN,
  MULTIPLAYER_SCREEN,
  CONTROLS_SCREEN,
  PAUSE_SCREEN,
  RACING_SCREEN,
  LOADING_SCREEN, // lowkey just the controls screen ? idk
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
  void drawMenu(BoundingBox region);
  void loadTextures();

};
