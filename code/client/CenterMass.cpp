#include "CenterMass.h"


// TODO :: this file has not been cleaned up and may not work when
// calling functions into client.cpp - will need to fix
// I just quickly factored out code from client.cpp without a lot of testing

// Provides a target (ideally the center of mass of a moving object)
// Renders the sphere where the transform for that object is
// This is the transform component version
void renderCMassSphere(TransformComponent& _target, TransformComponent& sphere_transform) {
	sphere_transform.setPosition(glm::vec3(_target.getTranslation().x, _target.getTranslation().y, _target.getTranslation().z));
}
// This is the PxTransform version as vehicle PhysX models use PxTransforms for their center of mass
void renderCMassSphere(PxTransform& _target, TransformComponent& sphere_transform) {
	sphere_transform.setPosition(glm::vec3(_target.p.x, _target.p.y, _target.p.z));
}

void setupSphere(ecs::Scene& mainScene) {
	ecs::Entity sphere_e = mainScene.CreateEntity();

	// Center of gravity sphere - used for debug
	RenderModel sphere_r = RenderModel();
	GraphicsSystem::importOBJ(sphere_r, "sphere.obj");
	sphere_r.setModelColor(glm::vec3(0.5f, 0.0f, 0.5f));
	mainScene.AddComponent(sphere_e.guid, sphere_r);
	TransformComponent sphere_t = TransformComponent(testCar.getVehicleRigidBody());
	sphere_t.setScale(glm::vec3(0.5f, 0.5f, 0.5f));
	mainScene.AddComponent(sphere_e.guid, sphere_t);
}

// Debug stuff for centre of mass - not working properly
		//c_mass_f.p.x = car_trans.getTranslation().x - tetherPole1_t.getTranslation().x;
		//c_mass_f.p.y = car_trans.getTranslation().y - tetherPole1_t.getTranslation().y;
		//c_mass_f.p.z = car_trans.getTranslation().z - tetherPole1_t.getTranslation().z;
		//c_mass_f.p.x = tetherPole1_t.getTranslation().x;
		//c_mass_f.p.y = tetherPole1_t.getTranslation().y;
		//c_mass_f.p.z = tetherPole1_t.getTranslation().z;
		//testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose = c_mass_f;


		// auto& center_of_mass = testCar.m_Vehicle.mPhysXParams.physxActorCMassLocalPose;
		// renderCMassSphere(center_of_mass, sphere_transform);

		// // Tether check to render the tether graphic properly
		// if (testCar.getCTethered()) {
		// 	updateTetherGraphic(car_trans, c_tether_points, testCar, tether_transform);
		// }
		// else {
		// 	tether_transform.setScale(glm::vec3(0.f, 0.f, 0.f));
		// }