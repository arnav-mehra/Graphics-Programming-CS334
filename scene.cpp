#include "Dimension.hpp"
#include "scene.h"

#include "_V3.hpp"
#include "_M33.hpp"
#include "_ppc.h"

using namespace std;

Scene* scene;

Scene::Scene() {
	scene = this;

	h = 480;
	w = 640;

	gui = new GUI();
	gui->show();
	ppc = new PPC(w, h);

	//// ADDS BOX
	MESH m;
	m.setAsBox();
	geometry.add_mesh(m);

	//// COOL DOTS
	//{
	//	V3 vec = V3(0.0f, 0.0f, -200.0f);
	//	M33 rot1 = M33(Dim::Y, 2.0f * PI / 64.0f);
	//	M33 rot2 = M33(Dim::X, 2.0f * PI / 64.0f);
	//	M33 rot(1);
	//	for (int theta = 0; theta < 64; theta++) {
	//		for (int phi = 0; phi < 64; phi++) {
	//			V3 new_vec = rot * vec;
	//			SPHERE s = SPHERE(new_vec, COLOR(255, 0, 0), 10);
	//			geometry.add_sphere(s);
	//			rot = rot1 * rot;
	//		}
	//		rot = rot2 * rot;
	//	}
	//}

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

void Scene::TransitionCamera() {
	fb->startThread();
}