#include "Obstacles.h"

#include "PxPhysicsAPI.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"


bool obstaclesOn = true;
std::vector<PxRigidStatic*> actors;
std::vector<int> guids;

void addRigidBody(physics::PhysicsSystem physicsSystem) {
	//PxBoxGeometry boxGeom(PxVec3(15.0f, 2.0f, 1.0f));

	//// create a rigid body with the box geometry
	//PxTransform transform(PxVec3(56.0f, 0.0f, 0.0f));
	//PxRigidDynamic* actor = PxCreateDynamic(
	//	*physicsSystem.m_Physics,
	//	transform,
	//	boxGeom,
	//	*physicsSystem.m_Material,
	//	100000.0f // This is the density value
	//);

	//// add the actor to the scene
	//physicsSystem.m_Scene->addActor(*actor);

	//PxBoxGeometry boxGeom2(PxVec3(15.0f, 2.0f, 1.0f));

	//// create a rigid body with the box geometry
	//PxTransform transform2(PxVec3(12.0f, 0.0f, 0.0f));
	//PxRigidDynamic* actor2 = PxCreateDynamic(
	//	*physicsSystem.m_Physics,
	//	transform2,
	//	boxGeom2,
	//	*physicsSystem.m_Material,
	//	100000.0f // This is the density value
	//);

	//// add the actor to the scene
	//physicsSystem.m_Scene->addActor(*actor2);


	// I'm having to do this multiple times with each a new instance
	//of the PxBoxGeometry and so on or else it will cause a memory address
	// Unable to be read issue

	PxBoxGeometry boxGeom(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform(PxVec3(56.0f, 0.0f, 0.0f));
	PxRigidStatic* actor = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform,
		boxGeom,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor);


	PxBoxGeometry boxGeom2(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform2(PxVec3(12.0f, 0.0f, 0.0f));
	PxRigidStatic* actor2 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform2,
		boxGeom2,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor2);

	PxBoxGeometry boxGeom3(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform3(PxVec3(35.0f, 0.0f, 60.0f));
	PxRigidStatic* actor3 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform3,
		boxGeom3,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor3);


	PxBoxGeometry boxGeom4(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform4(PxVec3(25.0f, 0.0f, 120.0f));
	PxRigidStatic* actor4 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform4,
		boxGeom4,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor4);


	PxBoxGeometry boxGeom5(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform5(PxVec3(45.0f, 0.0f, 160.0f));
	PxRigidStatic* actor5 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform5,
		boxGeom5,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor5);


	PxBoxGeometry boxGeom6(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform6(PxVec3(35.0f, 0.0f, -60.0f));
	PxRigidStatic* actor6 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform6,
		boxGeom6,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor6);

	PxBoxGeometry boxGeom7(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform7(PxVec3(25.0f, 0.0f, -120.0f));
	PxRigidStatic* actor7 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform7,
		boxGeom7,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor7);

	PxBoxGeometry boxGeom8(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform8(PxVec3(45.0f, 0.0f, -160.0f));
	PxRigidStatic* actor8 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform8,
		boxGeom8,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor8);


	PxBoxGeometry boxGeom9(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform9(PxVec3(-35.0f, 0.0f, -60.0f));
	PxRigidStatic* actor9 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform9,
		boxGeom9,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor9);

	PxBoxGeometry boxGeom10(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform10(PxVec3(-25.0f, 0.0f, -120.0f));
	PxRigidStatic* actor10 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform10,
		boxGeom10,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor10);

	PxBoxGeometry boxGeom11(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform11(PxVec3(-45.0f, 0.0f, -160.0f));
	PxRigidStatic* actor11 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform11,
		boxGeom11,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor11);

	PxBoxGeometry boxGeom12(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform12(PxVec3(-35.0f, 0.0f, 60.0f));
	PxRigidStatic* actor12 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform12,
		boxGeom12,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor12);

	PxBoxGeometry boxGeom13(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform13(PxVec3(-25.0f, 0.0f, 120.0f));
	PxRigidStatic* actor13 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform13,
		boxGeom13,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor13);

	PxBoxGeometry boxGeom14(PxVec3(15.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform14(PxVec3(-45.0f, 0.0f, 160.0f));
	PxRigidStatic* actor14 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform14,
		boxGeom14,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor14);


	// Tether Point
	PxBoxGeometry boxGeom15(PxVec3(1.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform15(PxVec3(0.0f, 1.0f, 162.0f));
	PxRigidStatic* actor15 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform15,
		boxGeom15,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor15);


	// Tether Point
	PxBoxGeometry boxGeom16(PxVec3(1.0f, 2.0f, 1.0f));

	// create a rigid body with the box geometry
	PxTransform transform16(PxVec3(0.0f, 1.0f, -162.0f));
	PxRigidStatic* actor16 = PxCreateStatic(
		*physicsSystem.m_Physics,
		transform16,
		boxGeom16,
		*physicsSystem.m_Material
	);

	// add the actor to the scene
	physicsSystem.m_Scene->addActor(*actor16);


	// If you want to add an object and rotate it - this is how you do it
	// You need to pass both the rotation and positon to PxTransform at the same time

	//// Ramp	
	//// Create a transform representing the initial position of the object
	//PxVec3 rampPosition(20.0f, -0.5f, 40.0f);

	//// Rotation of the object
	//PxQuat rampRotation(-0.12, PxVec3(1.0f, 0.0f, 0.0f));

	//PxTransform rampTransform(rampPosition, rampRotation);

	//PxBoxGeometry rampGeometry(PxVec3(5.0f, 1.0f, 5.0f));
	//PxRigidStatic* ramp = PxCreateStatic(*physicsSystem.m_Physics,
	//	rampTransform,
	//	rampGeometry,
	//	*physicsSystem.m_Material
	//);
	////*physics->createMaterial(0.5f, 0.5f, 0.1f)
	//physicsSystem.m_Scene->addActor(*ramp);

	actors.push_back(actor);
	actors.push_back(actor2);
	actors.push_back(actor3);
	actors.push_back(actor4);
	actors.push_back(actor5);
	actors.push_back(actor6);
	actors.push_back(actor7);
	actors.push_back(actor8);
	actors.push_back(actor9);
	actors.push_back(actor10);
	actors.push_back(actor11);
	actors.push_back(actor12);
	actors.push_back(actor13);
	actors.push_back(actor14);
	actors.push_back(actor15);
	actors.push_back(actor16);
}

void clearObstacles(physics::PhysicsSystem physicsSystem, ecs::Scene& mainScene) {
	// Removes the actors from the scene (Rigid Body)
	for (int i = 0; i < actors.size(); i++) {
		physicsSystem.m_Scene->removeActor(*actors.at(i));
	}
	actors.clear();
	
	// Change the ecs entity's transform scales
	// I originally wanted to delete the entities, but you can't do that without
	// messign up memory pointers in the ECS
	for (int i = 0; i < guids.size(); i++) {
		auto &transform = mainScene.GetComponent<TransformComponent>(guids.at(i));
		transform.setScale(glm::vec3(0.f));
		//mainScene.DestroyEntity(guids.at(i));
	}	
	//guids.clear();
}

void resetTransforms(ecs::Scene& mainScene) {
	// Resets the transforms of the components to their original values
	for (int i = 0; i < guids.size(); i++) {
		auto &transform = mainScene.GetComponent<TransformComponent>(guids.at(i));
		// This is hard-coded
		transform.setScale(glm::vec3(15.f, 2.f, 1.f));
	}
}

void setUpLogs(ecs::Scene &mainScene) {
	// Finish line logs
	ecs::Entity log1_e = mainScene.CreateEntity();
	RenderModel log1_r = RenderModel();
	GraphicsSystem::importOBJ(log1_r, "cube.obj");
	mainScene.AddComponent(log1_e.guid, log1_r);
	TransformComponent log1_t = TransformComponent();
	log1_t.setPosition(glm::vec3(56.f, 0.f, 0.f));
	log1_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log1_e.guid, log1_t);



	ecs::Entity log2_e = mainScene.CreateEntity();
	RenderModel log2_r = RenderModel();
	GraphicsSystem::importOBJ(log2_r, "cube.obj");
	mainScene.AddComponent(log2_e.guid, log2_r);
	TransformComponent log2_t = TransformComponent();
	log2_t.setPosition(glm::vec3(12.f, 0.f, 0.f));
	log2_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log2_e.guid, log2_t);

	// First quarter logs
	ecs::Entity log3_e = mainScene.CreateEntity();
	RenderModel log3_r = RenderModel();
	GraphicsSystem::importOBJ(log3_r, "cube.obj");
	mainScene.AddComponent(log3_e.guid, log3_r);
	TransformComponent log3_t = TransformComponent();
	log3_t.setPosition(glm::vec3(35.f, 0.f, 60.f));
	log3_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log3_e.guid, log3_t);

	ecs::Entity log4_e = mainScene.CreateEntity();
	RenderModel log4_r = RenderModel();
	GraphicsSystem::importOBJ(log4_r, "cube.obj");
	mainScene.AddComponent(log4_e.guid, log4_r);
	TransformComponent log4_t = TransformComponent();
	log4_t.setPosition(glm::vec3(25.f, 0.f, 120.f));
	log4_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log4_e.guid, log4_t);

	ecs::Entity log5_e = mainScene.CreateEntity();
	RenderModel log5_r = RenderModel();
	GraphicsSystem::importOBJ(log5_r, "cube.obj");
	mainScene.AddComponent(log5_e.guid, log5_r);
	TransformComponent log5_t = TransformComponent();
	log5_t.setPosition(glm::vec3(45.f, 0.f, 160.f));
	log5_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log5_e.guid, log5_t);

	// Second quarter logs
	ecs::Entity log6_e = mainScene.CreateEntity();
	RenderModel log6_r = RenderModel();
	GraphicsSystem::importOBJ(log6_r, "cube.obj");
	mainScene.AddComponent(log6_e.guid, log6_r);
	TransformComponent log6_t = TransformComponent();
	log6_t.setPosition(glm::vec3(-35.f, 0.f, 60.f));
	log6_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log6_e.guid, log6_t);

	ecs::Entity log7_e = mainScene.CreateEntity();
	RenderModel log7_r = RenderModel();
	GraphicsSystem::importOBJ(log7_r, "cube.obj");
	mainScene.AddComponent(log7_e.guid, log7_r);
	TransformComponent log7_t = TransformComponent();
	log7_t.setPosition(glm::vec3(-25.f, 0.f, 120.f));
	log7_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log7_e.guid, log7_t);

	ecs::Entity log8_e = mainScene.CreateEntity();
	RenderModel log8_r = RenderModel();
	GraphicsSystem::importOBJ(log8_r, "cube.obj");
	mainScene.AddComponent(log8_e.guid, log8_r);
	TransformComponent log8_t = TransformComponent();
	log8_t.setPosition(glm::vec3(-45.f, 0.f, 160.f));
	log8_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log8_e.guid, log8_t);

	//Third Quarter logs
	ecs::Entity log9_e = mainScene.CreateEntity();
	RenderModel log9_r = RenderModel();
	GraphicsSystem::importOBJ(log9_r, "cube.obj");
	mainScene.AddComponent(log9_e.guid, log9_r);
	TransformComponent log9_t = TransformComponent();
	log9_t.setPosition(glm::vec3(-35.f, 0.f, -60.f));
	log9_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log9_e.guid, log9_t);

	ecs::Entity log10_e = mainScene.CreateEntity();
	RenderModel log10_r = RenderModel();
	GraphicsSystem::importOBJ(log10_r, "cube.obj");
	mainScene.AddComponent(log10_e.guid, log10_r);
	TransformComponent log10_t = TransformComponent();
	log10_t.setPosition(glm::vec3(-25.f, 0.f, -120.f));
	log10_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log10_e.guid, log10_t);

	ecs::Entity log11_e = mainScene.CreateEntity();
	RenderModel log11_r = RenderModel();
	GraphicsSystem::importOBJ(log11_r, "cube.obj");
	mainScene.AddComponent(log11_e.guid, log11_r);
	TransformComponent log11_t = TransformComponent();
	log11_t.setPosition(glm::vec3(-45.f, 0.f, -160.f));
	log11_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log11_e.guid, log11_t);

	// Fourth Quarter Logs
	ecs::Entity log12_e = mainScene.CreateEntity();
	RenderModel log12_r = RenderModel();
	GraphicsSystem::importOBJ(log12_r, "cube.obj");
	mainScene.AddComponent(log12_e.guid, log12_r);
	TransformComponent log12_t = TransformComponent();
	log12_t.setPosition(glm::vec3(35.f, 0.f, -60.f));
	log12_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log12_e.guid, log12_t);

	ecs::Entity log13_e = mainScene.CreateEntity();
	RenderModel log13_r = RenderModel();
	GraphicsSystem::importOBJ(log13_r, "cube.obj");
	mainScene.AddComponent(log13_e.guid, log13_r);
	TransformComponent log13_t = TransformComponent();
	log13_t.setPosition(glm::vec3(25.f, 0.f, -120.f));
	log13_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log13_e.guid, log13_t);

	ecs::Entity log14_e = mainScene.CreateEntity();
	RenderModel log14_r = RenderModel();
	GraphicsSystem::importOBJ(log14_r, "cube.obj");
	mainScene.AddComponent(log14_e.guid, log14_r);
	TransformComponent log14_t = TransformComponent();
	log14_t.setPosition(glm::vec3(45.f, 0.f, -160.f));
	log14_t.setScale(glm::vec3(15.f, 2.f, 1.f));
	mainScene.AddComponent(log14_e.guid, log14_t);

	guids.push_back(log1_e.guid);
	guids.push_back(log2_e.guid);
	guids.push_back(log3_e.guid);
	guids.push_back(log4_e.guid);
	guids.push_back(log5_e.guid);
	guids.push_back(log6_e.guid);
	guids.push_back(log7_e.guid);
	guids.push_back(log8_e.guid);
	guids.push_back(log9_e.guid);
	guids.push_back(log10_e.guid);
	guids.push_back(log11_e.guid);
	guids.push_back(log12_e.guid);
	guids.push_back(log13_e.guid);
	guids.push_back(log14_e.guid);
}

void obstaclesImGui(ecs::Scene& mainScene, physics::PhysicsSystem physicsSystem) {
	if (ImGui::Checkbox("Obstacles", &obstaclesOn)) {
		if (obstaclesOn) {
			//setUpLogs(mainScene);
			resetTransforms(mainScene);
			addRigidBody(physicsSystem);
		}
		else {
			clearObstacles(physicsSystem, mainScene);
		}
	}
}
