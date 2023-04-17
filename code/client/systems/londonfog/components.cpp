#include "core/ecs.h"

#include "components.h"
#include "../../Input.h"
#include "../../entities/car/Car.h"


#include <sstream>
#include <string>


const float SCREEN_WIDTH = 1200.f;
const float SCREEN_HEIGHT = 800.f;

int n{ std::max(1, ControllerInput::getNumberPlayers())};

const ImVec2 AUTO_RESIZE = ImVec2(0.f,0.f);

ImGuiWindowFlags lfWindowFlags =
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoMove |				// text "window" should not move
  // ImGuiWindowFlags_NoResize |				// should not resize
  ImGuiWindowFlags_NoCollapse |			// should not collapse
  ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
  // ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
  // ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
  ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible


ImGuiWindowFlags emptyBackground =
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoMove |				// text "window" should not move
  // ImGuiWindowFlags_NoResize |				// should not resize
  ImGuiWindowFlags_NoCollapse |			// should not collapse
  ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
  // ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
  ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
  ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible


ImGuiWindowFlags controlsWindowFlags =
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoMove |				// text "window" should not move
  // ImGuiWindowFlags_NoResize |				// should not resize
  ImGuiWindowFlags_NoCollapse |			// should not collapse
  ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
  // ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
  // ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
  ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
  // ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible


ImGuiWindowFlags logoWindowFlags =
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoMove |				// text "window" should not move
  // ImGuiWindowFlags_NoResize |				// should not resize
  ImGuiWindowFlags_NoCollapse |			// should not collapse
  ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
  // ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
  ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
  ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
  // ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible


ImVec4 colorCodeToImguiVec(std::string colorCode, float opacity);
float hexToFloatByte(std::string h);



void loadFont(std::string fontname,  std::string simplename, float size, ImGuiIO& io, std::unordered_map<std::string,ImFont*>& fonts)
{
  ImFont* font = io.Fonts->AddFontFromFileTTF(fontname.c_str(), size);
  IM_ASSERT(font != NULL);
  fonts[simplename] = font;
}

void LondonFog::loadFonts()
{
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  //load fonts into ImGui
  loadFont("fonts/JockeyOne.ttf", "JockeyOne", 50.f, io, m_fonts);
  loadFont("fonts/JockeyOne.ttf", "JockeyOneSmall", 30.f, io, m_fonts);
  loadFont("fonts/JockeyOne.ttf", "JockeyOneXS", 15.f, io, m_fonts);
  loadFont("fonts/JockeyOne.ttf", "JockeyOneLarge", 130.f, io, m_fonts);
  loadFont("fonts/JockeyOne.ttf", "JockeyOneSpeed", 100.f, io, m_fonts);
  loadFont("fonts/JockeyOne.ttf", "JockeyOneXL", 150.f, io, m_fonts);
  loadFont("fonts/JockeyOne.ttf", "JockeyOneMedium", 70.f, io, m_fonts);


}





void LondonFog::setStyle()
{
  
  ImGuiStyle& style = ImGui::GetStyle();
  
  // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
  // style.Alpha = 1.0f;
  // style.FrameRounding = 3.0f;
  // style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  // style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  // style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
  // // style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  // style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  // style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
  // style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  // style.Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  // style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  // style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  // style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
  // style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
  // style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
  // style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  // style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  // style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
  // style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
  // style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  // // style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
  // style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
  // style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  // style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  // style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  // style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // // style.Colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  // // style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  // // style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  // style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
  // style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  // style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  // // style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
  // // style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  // // style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  // style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  // style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  // style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  // style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  // style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  // // style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

  // if( bStyleDark_ )
  // {
  //     for (int i = 0; i <= ImGuiCol_COUNT; i++)
  //     {
  //         ImVec4& col = style.Colors[i];
  //         float H, S, V;
  //         ImGui::ColorConvertRGBtoHSV( col.x, col.y, col.z, H, S, V );

  //         if( S < 0.1f )
  //         {
  //             V = 1.0f - V;
  //         }
  //         ImGui::ColorConvertHSVtoRGB( H, S, V, col.x, col.y, col.z );
  //         if( col.w < 1.00f )
  //         {
  //             col.w *= alpha_;
  //         }
  //     }
  // }
  // else
  // {
  //     for (int i = 0; i <= ImGuiCol_COUNT; i++)
  //     {
  //         ImVec4& col = style.Colors[i];
  //         if( col.w < 1.00f )
  //         {
  //             col.x *= alpha_;
  //             col.y *= alpha_;
  //             col.z *= alpha_;
  //             col.w *= alpha_;
  //         }
  //     }
  // }


}


std::string getRankSuffix(int rank)
{
  int ending = rank % 10;
  switch (ending)
  {
    case 1:
      return "st";
    case 2:
      return "nd";
    case 3:
      return "rd";
    default:
      return "th";
  } 

}

void LondonFog::drawHUD(Guid carGuid, ecs::Scene& scene, BoundingBox region, RaceTracker& raceSystem)
{

  std::vector<std::string> hudFonts; 



  if (n != 1)
  {
    hudFonts.push_back("JockeyOneXS");
    hudFonts.push_back("JockeyOneSmall");
  }
  else {
    hudFonts.push_back("JockeyOne");
    hudFonts.push_back("JockeyOneSpeed");
  }

  // get the associated data  

  // start the imgui calls

  // setup the style

  if (m_status != RACING_SCREEN)
  {
    return;
  } 



  // start with current speed
  // lower left corner.

  Car& car = scene.GetComponent<Car>(carGuid);

  ProgressTracker& pt = scene.GetComponent<ProgressTracker>(carGuid);

  if (pt.isFinished)
  {
    // display race complete message!
    float size = region.w / 2; 
    ImVec2 imageSize{size * 1.60f, size}; // enforce aspect ratio
    ImVec2 pos = region.getRelativeCenter(imageSize);

    ImGui::SetNextWindowSize(ImVec2(0.f,0.f)); // scale to fill content (img size in this case)
    ImGui::SetNextWindowPos(pos);
    ImGui::Begin("finished", false, emptyBackground);

    m_texts["finished"].Render(imageSize);

    ImGui::End();
    return;
  }


  // wrong way prompt

  float size = region.w / 3;

  std::string label_hud_msg = std::string("hud_message_") + std::to_string(carGuid);


  if (car.isWrongWay())
  {
    ImVec2 imageSize{size * 1.60f, size}; // enforce aspect ratio
    ImVec2 pos = region.getRelativeCenter(imageSize);

    pos.y += 0.23f * static_cast<float>(region.h);

    ImGui::SetNextWindowSize(ImVec2(0.f,0.f)); // scale to fill content (img size in this case)
    ImGui::SetNextWindowPos(pos);
    ImGui::Begin(label_hud_msg.c_str(), false, emptyBackground);

    m_texts["wrongway"].Render(imageSize);

    ImGui::End();
  } else if (car.m_timeSinceLastRamp < 1.8f)
  {
    ImVec2 imageSize{size * 1.60f, size}; // enforce aspect ratio
    ImVec2 pos = region.getRelativeCenter(imageSize);

    pos.y += 0.23f * static_cast<float>(region.h);

    ImGui::SetNextWindowSize(ImVec2(0.f,0.f)); // scale to fill content (img size in this case)
    ImGui::SetNextWindowPos(pos);
    ImGui::Begin(label_hud_msg.c_str(), false, emptyBackground);

    m_texts["ramp"].Render(imageSize);

    ImGui::End();
  } else if (car.m_timeSinceLastBoost < 1.2f)
  {
    ImVec2 imageSize{size * 1.60f, size}; // enforce aspect ratio
    ImVec2 pos = region.getRelativeCenter(imageSize);

    pos.y += 0.23f * static_cast<float>(region.h);

    ImGui::SetNextWindowSize(ImVec2(0.f,0.f)); // scale to fill content (img size in this case)
    ImGui::SetNextWindowPos(pos);
    ImGui::Begin(label_hud_msg.c_str(), false, emptyBackground);

    m_texts["pillar"].Render(imageSize);

    ImGui::End();
  }

  // hud 

  float startY{0.73f};
  float startX{0.015f};

  ImVec2 bottomLeft = region.getCorner(BOTTOM_LEFT);
  ImVec2 bottomRight = region.getCorner(BOTTOM_RIGHT);


  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{18.f, 12.f});
  ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#EF1A1A", 0.65f));
  ImGui::PushFont(m_fonts[hudFonts[0]]);

  std::string label_hud = std::string("hud_") + std::to_string(carGuid);

  ImGui::SetNextWindowSize(ImVec2(0.f,0.f));
  // ImGui::SetNextWindowPos(ImVec2(bottomLeft.x + startX * region.w, region.y + startY * region.h));
  std::string label_speed = std::string("speed_") + std::to_string(carGuid);

  ImGui::Begin(label_speed.c_str(),false,lfWindowFlags);

  ImGui::SetWindowFontScale(1.f);

  auto carSpeed = car.carSpeed();

  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Current Speed");
  ImGui::PushFont(m_fonts[hudFonts[1]]);
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.0f", clamp((carSpeed * (carSpeed / 10.f)), 0.f, 500.f));
  // ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%.0f", (carSpeed));
  ImGui::PopFont();
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "u/s");

  ImVec2 windowSize = ImGui::GetWindowSize();
  // make the clamp relative to actual region size
  // make font scale to region size

  float offsetCorner = (region.w / 28);

  ImGui::SetWindowPos(ImVec2(bottomLeft.x + offsetCorner, bottomLeft.y - windowSize.y - offsetCorner));

  ImGui::SetWindowSize(ImVec2(clamp(windowSize.x,70.f, 100.f), windowSize.y));

  ImGui::End();

  float itemWidth{0.10f};
  float startOff{0.7f};
  float gap{15.f};

  // then do the rank
  ImGui::SetNextWindowSize(ImVec2(0.f,0.f));
  // ImGui::SetNextWindowPos(ImVec2((region.x + region.w) - (region.w * startX), region.y + startY * region.h));
  std::string label_rank = std::string("rank_") + std::to_string(carGuid);
  
  ImGui::Begin(label_rank.c_str(),false,lfWindowFlags);
  ImVec2 v = ImGui::GetWindowSize();
  ImGui::SetWindowPos(ImVec2(bottomRight.x - offsetCorner - v.x, bottomRight.y - v.y - offsetCorner));


  int ranking = raceSystem.getRanking(carGuid);
  ImGui::BeginGroup();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Rank");
  ImGui::PushFont(m_fonts[hudFonts[1]]);
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", ranking);
  ImGui::PopFont();
  ImGui::SameLine();
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), getRankSuffix(ranking).c_str());
  ImGui::EndGroup();

  int spacingCount = 7;
  if (n > 2)
  {
    spacingCount = 3;
  }

  if (n == 2)
  {
    spacingCount = 2;
  }

  for (int i =0; i < spacingCount; i++)
  {
    ImGui::SameLine();
    ImGui::Spacing();
  }

  ImGui::SameLine();

  ImGui::BeginGroup();  
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Lap");
  ImGui::PushFont(m_fonts[hudFonts[1]]);
  ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d/%d", raceSystem.getLapCount(carGuid), raceSystem.MAX_LAPS);
  ImGui::PopFont();
  ImGui::EndGroup();

  ImGui::End();

  // next, do the lapcount
  // ImGui::SetNextWindowSize(ImVec2(region.w * itemWidth,0.f));
  // ImGui::SetNextWindowPos(ImVec2(region.x + startOff * region.w + (region.w * itemWidth) + gap, region.y + startY * region.h));
  // std::string label_lap = std::string("lapcount_") + std::to_string(carGuid);
  
  // ImGui::Begin(label_lap.c_str(),false,lfWindowFlags);


  // ImGui::End();


  ImGui::PopFont();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  
  std::string label_hud_label = std::string("hud_label_") + std::to_string(carGuid);
  ImVec2 tl = region.getCorner(TOP_LEFT);
  ImGui::SetNextWindowPos(ImVec2(tl.x + (region.w / 10), tl.y + (region.h / 10)));

  ImGui::Begin(label_hud.c_str(), false, lfWindowFlags);

  ImGui::PushFont(m_fonts[hudFonts[0]]);
  ImGui::Text("Playing as: %s", car.m_name.c_str());
  ImGui::PopFont();
  
  ImGui::End();

  // pop the style



}




void LondonFog::drawMenu(BoundingBox region, std::function<void(void)> resetCallback, std::function<void(int)> cameraCallback, std::function<void()> togglePauseCallback, ecs::Scene& scene, RaceTracker& raceSystem)
{
  // push the style options

  if (m_status == MAIN_SCREEN)
  {

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{30.f, 20.f});
    ImGui::PushFont(m_fonts["JockeyOne"]);


    ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#000000", 0.65f));
    // draw our names :D

    ImVec2 blCorner = region.getCorner(Corner::BOTTOM_LEFT);
    ImGui::SetNextWindowSize(AUTO_RESIZE);
    ImGui::Begin("authors",false, lfWindowFlags);



    ImGui::PushFont(m_fonts["JockeyOneMedium"]);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "By Sam, Elise, Dylan and Beau");
    ImGui::PopFont();


    ImVec2 v = ImGui::GetWindowSize();
    ImVec2 textOffset = {0.f, 40.f};
    ImGui::SetWindowPos(ImVec2(blCorner.x + textOffset.x, blCorner.y - v.y - textOffset.y));

    ImGui::End();

    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#000000", 0.0f));

    // draw "Maximus Overdrive" (game title)
    ImVec2 trCorner = region.getCorner(Corner::TOP_RIGHT);
    ImGui::SetNextWindowSize(AUTO_RESIZE);
    ImGui::Begin("gametitle",false, logoWindowFlags);

    // ImGui::PushFont(m_fonts["JockeyOneXL"]);
    // ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Maximus");
    // for (int i =0; i < 10; i++)
    // {
    //   ImGui::Spacing();
    //   ImGui::SameLine();
    // }
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Overdrive");
    // ImGui::PopFont();

    float size = 245; 
    ImVec2 imageSize{size * 2.71f, size}; // enforce aspect ratio

    m_texts["logo"].Render(imageSize);


    v = ImGui::GetWindowSize();
    textOffset = {0.f, 50.f};
    ImGui::SetWindowPos(ImVec2(trCorner.x - v.x, trCorner.y + textOffset.y));

    ImGui::End();


    ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#bf0d0d", 0.65f));

    ImVec2 brCorner = region.getCorner(Corner::BOTTOM_RIGHT);
    ImGui::SetNextWindowSize(AUTO_RESIZE);
    ImGui::Begin("main_menu_buttons",false, lfWindowFlags);

    ImGui::PushFont(m_fonts["JockeyOneMedium"]);
    
    ImGui::PushStyleColor(ImGuiCol_Button,        colorCodeToImguiVec("#770909", 0.65f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorCodeToImguiVec("#770909", 0.87f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  colorCodeToImguiVec("#300303", 0.65f));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15.f,15.f));

    if (ImGui::Button("Singleplayer"))
    {
      cameraCallback(n); // need to rig the controller setup
      resetCallback();
      resetCallback();
      m_status = RACING_SCREEN;
    }

    ImGui::Spacing();
    if (ImGui::Button("Multiplayer"))
    {
      m_status = MULTIPLAYER_SCREEN;
    }
    ImGui::Spacing();
    if (ImGui::Button("Controls"))
    {
      m_status = CONTROLS_SCREEN;
    }
    ImGui::Spacing();

    ImGui::PopStyleVar();

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    ImGui::PopStyleColor();


    ImGui::PopFont();


    v = ImGui::GetWindowSize();
    textOffset = {40.f, 40.f};
    ImGui::SetWindowPos(ImVec2(brCorner.x - v.x - textOffset.x, brCorner.y - v.y - textOffset.y));

    ImGui::End();



    ImGui::PopStyleVar();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.f, 0.f});


    ImVec2 menuSize = ImVec2(static_cast<float>(region.w + 2), static_cast<float>(region.h + 2));
    ImGui::SetNextWindowSize(menuSize);
    ImGui::SetNextWindowPos(ImVec2(-1.f,-1.f));


    ImGui::Begin("mainmenu", false, emptyBackground);

    // take up the entire screen
    m_texts["mainmenu"].Render(menuSize);

    ImGui::End();


    // now draw the other elements of the screen.

    // should take up right side of screen
    // all in one window? or should I split it up??
    
    ImGui::Begin("gametitle", false, lfWindowFlags);

    ImGui::End();

      
    // pop the style options
    ImGui::PopFont();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

  } else if (m_status == MULTIPLAYER_SCREEN)
  {

    ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#000000", 1.00f));
    ImGui::PushFont(m_fonts["JockeyOneMedium"]);


    ImGui::PushStyleColor(ImGuiCol_Button,        colorCodeToImguiVec("#FFFFFF", 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorCodeToImguiVec("#999999", 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  colorCodeToImguiVec("#4d4d4d", 1.00f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.f, 40.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20.f, 20.f));

    ImGui::SetNextWindowPos(region.getCorner(TOP_LEFT));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(region.w), static_cast<float>(region.h)));
    // draw one big fat window with lots of padding, radio buttons
    ImGui::Begin("multiplayer_menu", false, lfWindowFlags);
    
    ImGui::PushStyleColor(ImGuiCol_Text,        colorCodeToImguiVec("#000000", 1.00f));
    ImGui::PushFont(m_fonts["JockeyOne"]);
    if (ImGui::Button("Back to main menu"))
    {
      m_status = MAIN_SCREEN;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();

    ImGui::PushFont(m_fonts["JockeyOneXL"]);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Start a multiplayer race");
    ImGui::PopFont();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Plug in up to 4 controllers to play! (Do NOT unplug any!)");
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20.f,20.f));
    
    int numPlayers = std::max(1,ControllerInput::getNumberPlayers());
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "There are currently %d controllers detected.", numPlayers);

    for (int i = 0; i < numPlayers; i++)
    {
      int corrected = i + 1;
      ImGui::RadioButton(std::to_string(corrected).c_str(), &n, corrected);
      if (i != 3 && corrected < numPlayers)
      {
        ImGui::SameLine();
      }
    }

    // need to set the camera vars here...

    ImGui::PopStyleVar();

    // pop window padding, item spacing
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    ImGui::PushStyleColor(ImGuiCol_Text,        colorCodeToImguiVec("#000000", 1.00f));
    if (ImGui::Button("Start Racing!"))
    {
      cameraCallback(n);
      resetCallback();
      m_status = RACING_SCREEN;
    }
    ImGui::PopStyleColor();

    ImGui::PopFont();
    ImGui::End();

    // button colors
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    // window color
    ImGui::PopStyleColor();


    // draw the multiplayer screen
  } else if (m_status == CONTROLS_SCREEN)
  {
 
    ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#000000", 1.00f));

    ImGui::PushStyleColor(ImGuiCol_Button,        colorCodeToImguiVec("#FFFFFF", 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorCodeToImguiVec("#999999", 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  colorCodeToImguiVec("#4d4d4d", 1.00f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.f, 20.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(20.f, 0.f));

    ImGui::SetNextWindowPos(region.getCorner(TOP_LEFT));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(region.w), static_cast<float>(region.h)));
    // draw one big fat window with lots of padding, radio buttons
    ImGui::Begin("multiplayer_menu", false, controlsWindowFlags);
    
    ImGui::PushStyleColor(ImGuiCol_Text,        colorCodeToImguiVec("#000000", 1.00f));
    ImGui::PushFont(m_fonts["JockeyOne"]);
    if (ImGui::Button("Back to main menu"))
    {
      m_status = MAIN_SCREEN;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor();

    ImGui::PushFont(m_fonts["JockeyOneXL"]);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Controls");
    ImGui::PopFont();


    ImGui::PushFont(m_fonts["JockeyOne"]);
    ImGui::Text("NOTE: Ramps and jumping over pillars will also give you a nice boost!");
    ImGui::PopFont();
    ////// controls image

    float size = 500; 
    ImVec2 imageSize{size * 2.18f, size}; // enforce aspect ratio

    m_texts["controls"].Render(imageSize);

    //////


    ImGui::End();

    // pop window padding, item spacing
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    // button colors
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    // window color
    ImGui::PopStyleColor();


  
  } else if (m_status == PAUSE_SCREEN)
  {
    // draw the pause screen

    // draw a window in the middle of the screen
    // with "resume?"


    ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#bf0d0d", 0.65f));

    ImGui::SetNextWindowSize(AUTO_RESIZE);
    ImGui::Begin("pause_menu",false, lfWindowFlags);

    ImGui::PushFont(m_fonts["JockeyOneLarge"]);
    ImGui::Text("Game Paused");
    ImGui::PopFont();

    ImGui::PushFont(m_fonts["JockeyOneMedium"]);
    
    ImGui::PushStyleColor(ImGuiCol_Button,        colorCodeToImguiVec("#770909", 0.65f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorCodeToImguiVec("#770909", 0.87f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  colorCodeToImguiVec("#300303", 0.65f));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15.f,15.f));

    if (ImGui::Button("Resume Race"))
    {
      // unpause the game
      m_status = RACING_SCREEN;
      togglePauseCallback();
    }

    ImGui::Spacing();
    if (ImGui::Button("Reset Race"))
    {
      // reset
      resetCallback();
      // unpause
      togglePauseCallback();
      m_status = RACING_SCREEN;

    }
    
    ImGui::Spacing();
    if (ImGui::Button("End Race"))
    {
      // check the controls ? 
      m_status = MAIN_SCREEN;
    }
    ImGui::Spacing();
    // pop frame padding
    ImGui::PopStyleVar();

    // pop button colors
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    // pop window color
    ImGui::PopStyleColor();

    // pop medium font
    ImGui::PopFont();

    ImVec2 v = ImGui::GetWindowSize();
    ImVec2 center = region.getRelativeCenter(v);

    ImGui::SetWindowPos(ImVec2(center.x, center.y));

    ImGui::End();

  } else if (m_status == RESULTS_SCREEN)
  {
    // draw the results screen.



    ImGui::PushStyleColor(ImGuiCol_WindowBg, colorCodeToImguiVec("#000000", 0.65f));

    ImGui::SetNextWindowSize(AUTO_RESIZE);
    ImGui::Begin("pause_menu",false, lfWindowFlags);

    ImGui::PushFont(m_fonts["JockeyOneLarge"]);
    ImGui::Text("Race Complete!");
    ImGui::PopFont();

    ImGui::PushFont(m_fonts["JockeyOneSmall"]);

    ImGui::PushFont(m_fonts["JockeyOneMedium"]);
    ImGui::Text("Final Rankings");
    ImGui::PopFont();

    ImGui::Spacing();


    auto getName = [](Guid g, ecs::Scene& scene)
    {
      Car& car = scene.GetComponent<Car>(g);
      return car.m_name.c_str();
    };

    std::vector<Guid>& orderedRankings = raceSystem.getOrderedRankings();
    for (int i = 0; i <  orderedRankings.size(); i++)
    {
      ImGui::Text("%d. %s", i+1, getName(orderedRankings[i], scene));
    }

    // ImGui::Text("1. Player 1");
    // ImGui::Text("2. Player 1");
    // ImGui::Text("3. Player 1");
    // ImGui::Text("4. Player 1");
    // ImGui::Text("5. Player 1");
    // ImGui::Text("6. Player 1");
    // ImGui::Text("7. Player 1");
    // ImGui::Text("8. Player 1");
    // ImGui::Text("9. Player 1");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button,        colorCodeToImguiVec("#000000", 0.65f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorCodeToImguiVec("#000000", 0.87f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  colorCodeToImguiVec("#111111", 0.65f));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f,5.f));


    ImGui::PushFont(m_fonts["JockeyOne"]);
  
    if (ImGui::Button("Back To Main Menu"))
    {
      // check the controls ? 
      m_status = MAIN_SCREEN;
    }

    ImGui::PopFont();

    // pop frame padding
    ImGui::PopStyleVar();

    // pop button colors
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    
    ImGui::PopFont();


    // pop window color
    ImGui::PopStyleColor();



    ImVec2 v = ImGui::GetWindowSize();
    ImVec2 center = region.getRelativeCenter(v);

    ImGui::SetWindowPos(ImVec2(center.x, center.y));

    ImGui::End();


  }




  
}


ImVec4 colorCodeToImguiVec(std::string colorCode, float opacity)
{

  // #abcdef
  // 0123456

  // Function to parse HTML color code and return RGBA vector

  auto red_str    = colorCode.substr(1,2);
  auto green_str  = colorCode.substr(3,2);
  auto blue_str   = colorCode.substr(5,2);

  // color - 65

  // Extract red, green, blue values from integer
  float red   = hexToFloatByte(red_str);
  float green = hexToFloatByte(green_str);
  float blue  = hexToFloatByte(blue_str);

  // Add values to vector and return

  return ImVec4(red, green, blue, opacity);
}


float hexToFloatByte(std::string h)
{
  unsigned int rgb;
  std::stringstream ss;
  ss << std::hex << h;
  ss >> rgb;
  return static_cast<float>(rgb) / 255.0f;
}



// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void LondonFog::loadTextures()
{
  UITexture wrongWay{"textures/wrongway.png"};
  m_texts["wrongway"] = wrongWay;

  UITexture mainmenu{"textures/mainmenubkg.png"};
  m_texts["mainmenu"] = mainmenu;

  UITexture text{"textures/controls-flipped.png"};
  m_texts["controls"] = text;

  UITexture t{"textures/logo-flipped.png"};
  m_texts["logo"] = t;

  UITexture rampBoost{"textures/ramp_boost.png"};
  m_texts["ramp"] = rampBoost;

  UITexture pillarBoost{"textures/pillarboost.png"};
  m_texts["pillar"] = pillarBoost;

  UITexture finished{"textures/finished.png"};
  m_texts["finished"] = finished;
  // UITexture black{"textures/black.png"};
  // m_texts["black"] = black;


}
