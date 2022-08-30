#include <iostream>
#include <fstream>

#include "Dimension.hpp"
#include "scene.h"

#include "_V3.hpp"
#include "_M33.hpp"

using namespace std;

Scene* scene;

Scene::Scene() {
	scene = this;

	h = 480;
	w = 640;
	gui = new GUI();
	gui->show();
	
	perspective = M33(Dim::X, -0.1f) * M33(Dim::Y, 0.1f);
	origin = V3((float) w * 0.5f, (float) h * 0.5f, 0.0f);
	geometry = GEOMETRY();

	int u0 = 16, v0 = 40;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("SW framebuffer");
	fb->show();
	fb->redraw();
	gui->uiw->position(u0+w+u0, v0);
}

void Scene::DBG() {
	cerr << "INFO: pressed DBG button on GUI" << endl;
}

void Scene::NewButton() {
	cerr << "INFO: pressed New button on GUI" << endl;
}