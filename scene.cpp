#include "gui.hpp"
#include "ppc.hpp"
#include "scene.hpp"

using namespace std;

Scene* scene;

Scene::Scene() {
	scene = this;

	h = 480;
	w = 640;

	gui = new GUI();
	gui->show();
	ppc = new PPC();

	// ADDS BOX
	//{
	//	MESH m;
	//	m.setAsBox(V3(50.0f, 50.0f, -300.0f), 50.0f);
	//	m.fill = true;
	//	geometry.add_mesh(m);
	//}

	// SPHERE WIRE FRAME
	//{
	//	MESH m;
	//	m.setAsSphere(V3(0.0f, 0.0f, -1000.0f), 20U, 200.0f);
	//	geometry.add_mesh(m);
	//}

	// SPHERE WIRE FRAME
	{
		MESH m;
		m.setAsCylinder(V3(0.0f, 0.0f, -500.0f), 20U, 200.0f, 100.0f);
		geometry.add_mesh(m);
	}

	// ADD LIGHT SOURCE
	{
		LIGHT li = LIGHT(V3(0.0f, 0.0f, -500.0f), V3(1.0f, 1.0f, 1.0f), COLOR(255, 255, 255), 1.4f);
		geometry.add_light(li);
	}

	rotation_axis1 = V3(0.0f, -1.0f, -1.0f);
	rotation_axis2 = V3(0.0f, 1.0f, 1.0f);

	int u0 = 16, v0 = 40;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("SW framebuffer");
	fb->show();
	fb->redraw();
	gui->uiw->position(u0 + w + u0, v0);	
}

void Scene::LoadTxtButton() {
	ppc->LoadTxt();
	fb->redraw();
}

void Scene::SaveTxtButton() {
	ppc->SaveAsTxt();
	fb->redraw();
}

void Scene::LoadBinButton() {
	geometry.meshes[SEL_MESH].LoadBin();
	fb->redraw();
}

void Scene::SaveBinButton() {
	geometry.meshes[SEL_MESH].SaveAsBin();
	fb->redraw();
}

void Scene::RotationButton() {
	geometry.meshes[SEL_MESH].rotate(rotation_axis1, rotation_axis2, 0.1f);
	fb->redraw();
}

void Scene::TransitionCamera() {
	fb->startThread();
}

void Scene::LightLeft() {
	geometry.lights[SEL_LIGHT].source[Dim::X] -= 0.1f;
	fb->redraw();
}

void Scene::LightRight() {
	geometry.lights[SEL_LIGHT].source[Dim::X] += 0.1f;
	fb->redraw();
}

void Scene::LightUp() {
	geometry.lights[SEL_LIGHT].source[Dim::Y] -= 0.1f;
	fb->redraw();
}

void Scene::LightDown() {
	geometry.lights[SEL_LIGHT].source[Dim::Y] += 0.1f;
	fb->redraw();
}

void Scene::LightFront() {
	geometry.lights[SEL_LIGHT].source[Dim::Z] += 0.1f;
	fb->redraw();
}

void Scene::LightBack() {
	geometry.lights[SEL_LIGHT].source[Dim::Z] -= 0.1f;
	fb->redraw();
}