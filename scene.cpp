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
	frame = 0;
	hfov = 60.0f;

	gui = new GUI();
	gui->show();

	origin = V3((float)w * 0.5f, (float)h * 0.5f, 0.0f);
	ppc = new PPC(hfov, w, h);

	{
		SPHERE circle = SPHERE(
			V3(-10.0f, 10.0f, 20.0f),
			COLOR(0, 255, 0),
			40
		);
		geometry.add_sphere(circle);
	}
	{
		SPHERE circle = SPHERE(
			V3(10.0f, -10.0f, 20.0f),
			COLOR(0, 255, 0),
			40
		);
		geometry.add_sphere(circle);
	}

	int u0 = 16, v0 = 40;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("SW framebuffer");
	fb->show();

	fb->SetBGR(0);
	fb->applyGeometry();
	fb->redraw();

	gui->uiw->position(u0+w+u0, v0);
}

void Scene::LoadTiffButton() {
	fb->LoadTiff();
	fb->redraw();
}

void Scene::SaveTiffButton() {
	fb->SaveAsTiff();
}

void Scene::TranslateImage() {
	fb->startThread();
}