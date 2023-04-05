// TODO(beau): sdl cleanup:
//             There are a number of sdl things we could clean up, we simply need to figure out when/where
//             - SDL_Quit()
//             - clean up OpenGL context (will we switch contexts?)
//             - clean up the window?
//             - other things iirc
//             IMO these things will become more clear once we know more about the architecture of our program.
//             For now, we can leave these things as we're not leaking memory and the OS is cleaning up after us.
//             Note that the glfw code also didn't do a great job of cleaning up after itself
#include "Window.h"

#include <iostream>

void sdl_error_handler(int sdl_return) {
	if (!sdl_return) return;
	printf("SDL error: ", SDL_GetError());
	SDL_Quit();
	exit(1);
}


Window::Window(int width, int height, const char* title)
{
	if (!Backend::Initialize("Maximus Overdrive", 1200, 800, false))
		std::cout << "Failed to initalize backend\n";
	Rml::SetSystemInterface(Backend::GetSystemInterface());
	Rml::SetRenderInterface(Backend::GetRenderInterface());
	Rml::Initialise();
	rmlContext = Rml::CreateContext("main", Rml::Vector2i(1200, 800));
	if (!rmlContext) {
		Rml::Shutdown();
		Backend::Shutdown();
		std::cout << "Failed to create RmlUI context\n";
	}
	
	Rml::Debugger::Initialise(rmlContext);
	//load the sample document
	if (!Rml::LoadFontFace("fonts/Cabal-w5j3.ttf"))
		std::cout << "failed to load font\n";
	document = rmlContext->LoadDocument("Rml/demo.rml");
	if (document)
		document->Show();
	else
		std::cout << "Failed to load document\n";
	document->Hide();

	Rml::Log::Message(Rml::Log::LT_DEBUG, "Test warning.");

	std::cout << "finished initalizing window\n\n";
}


glm::ivec2 Window::getPos() const {
	int x, y = 0;
	//SDL_GetWindowPosition(window.get(), &x, &y);
	return glm::ivec2(x, y);
}


glm::ivec2 Window::getSize() const {
	int w, h = 0;
	//SDL_GetWindowSize(window.get(), &w, &h);
	return glm::ivec2(w, h);
}

void Window::RenderAndSwap()
{
	
	Backend::ProcessEvents(rmlContext, static_cast<KeyDownCallback>(&Window::ProcessKeyDownShortcuts));

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glActiveTexture(GL_TEXTURE0);
	rmlContext->Update();
	Backend::BeginFrame();
	rmlContext->Render();
	glDisable(GL_BLEND);
#define ImGUI_Enabled
#ifdef ImGUI_Enabled
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_FRAMEBUFFER_SRGB);
	ImGui::Render();
	glViewport(0, 0, 1200, 800);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glEnable(GL_FRAMEBUFFER_SRGB);
#endif
	//swap the buffers
	Backend::PresentFrame();
}

Rml::ElementDocument* Window::document = nullptr;
bool Window::ProcessKeyDownShortcuts(Rml::Context* context, Rml::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority)
{
	if (!context)
		return true;

	// Result should return true to allow the event to propagate to the next handler.
	bool result = false;

	// This function is intended to be called twice by the backend, before and after submitting the key event to the context. This way we can
	// intercept shortcuts that should take priority over the context, and then handle any shortcuts of lower priority if the context did not
	// intercept it.
	if (priority)
	{
		// Priority shortcuts are handled before submitting the key to the context.

		// Toggle debugger and set dp-ratio using Ctrl +/-/0 keys.
		if (key == Rml::Input::KI_F8)
		{
			Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
		}
		else if (key == Rml::Input::KI_F7) {
			if (document->IsVisible())
				document->Hide();
			else
				document->Show();
		}
		else if (key == Rml::Input::KI_0 && key_modifier & Rml::Input::KM_CTRL)
		{
			context->SetDensityIndependentPixelRatio(native_dp_ratio);
		}
		else if (key == Rml::Input::KI_1 && key_modifier & Rml::Input::KM_CTRL)
		{
			context->SetDensityIndependentPixelRatio(1.f);
		}
		else if ((key == Rml::Input::KI_OEM_MINUS || key == Rml::Input::KI_SUBTRACT) && key_modifier & Rml::Input::KM_CTRL)
		{
			const float new_dp_ratio = Rml::Math::Max(context->GetDensityIndependentPixelRatio() / 1.2f, 0.5f);
			context->SetDensityIndependentPixelRatio(new_dp_ratio);
		}
		else if ((key == Rml::Input::KI_OEM_PLUS || key == Rml::Input::KI_ADD) && key_modifier & Rml::Input::KM_CTRL)
		{
			const float new_dp_ratio = Rml::Math::Min(context->GetDensityIndependentPixelRatio() * 1.2f, 2.5f);
			context->SetDensityIndependentPixelRatio(new_dp_ratio);
		}
		else
		{
			// Propagate the key down event to the context.
			result = true;
		}
	}
	else
	{
		// We arrive here when no priority keys are detected and the key was not consumed by the context. Check for shortcuts of lower priority.
		if (key == Rml::Input::KI_R && key_modifier & Rml::Input::KM_CTRL)
		{
			for (int i = 0; i < context->GetNumDocuments(); i++)
			{
				Rml::ElementDocument* document = context->GetDocument(i);
				const Rml::String& src = document->GetSourceURL();
				if (src.size() > 4 && src.substr(src.size() - 4) == ".rml")
				{
					document->ReloadStyleSheet();
				}
			}
		}
		else
		{
			result = true;
		}
	}

	return result;
}