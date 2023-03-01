void RUN_GRAPHICS_TEST_BENCH() {
	SDL_Init(SDL_INIT_EVERYTHING); // initialize all sdl systems
	Window window(1200, 800, "Maximus Overdrive");
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//initalize the bare minimum ecs
	ecs::Scene mainScene;
	GraphicsSystem gs(window);

	// Car Entity
	ecs::Entity car_e = mainScene.CreateEntity();
	RenderModel car_r = RenderModel();
	GraphicsSystem::importOBJ(car_r, "utah_teapot.obj");
	car_r.setModelColor(glm::vec3(0.5f, 0.5f, 0.f));
	mainScene.AddComponent(car_e.guid, car_r);
	TransformComponent car_t = TransformComponent();
	car_t.setPosition(glm::vec3(5, 0, 0));
	mainScene.AddComponent(car_e.guid, car_t);

	bool quit = false;
	while (!quit) {
		//polls all pending input events until there are none left in the queue
		while (SDL_PollEvent(&window.event)) {
			ImGui_ImplSDL2_ProcessEvent(&window.event);
			if (window.event.type == SDL_QUIT)
				quit = true;

			if (window.event.type == SDL_KEYDOWN) {
				switch (window.event.key.keysym.sym) {
				case SDLK_ESCAPE:	// (Pressing escape closes the window, useful for fullscreen);
					quit = true;
					break;
				default:
					break;
				};
			}

			//pass the event to the camera
			gs.input(window.event, 0);
		}

		//process the frame
		gs.Update(mainScene, 0.0f);
		glDisable(GL_FRAMEBUFFER_SRGB);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		gs.ImGuiPanel();

		//ImGui
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_Quit();
	exit(0);
}