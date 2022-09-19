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
	ppc = new PPC(hfov, w, h);

	{
		int interval = 1000;
		M33 rx = M33(Dim::X, 0.1);
		M33 ry = M33(Dim::X, 0.1);
		M33 rz = M33(Dim::X, 0.1);
		V3 v1 = V3(1000, 0, 0);
		for (int x = -interval; x <= interval; x += interval) {
			V3 res;
			if (ppc->Project(v1, res)) {
				cout << res;
				SPHERE s = SPHERE(res, COLOR(255, 0, 0), 10);
				geometry.add_sphere(s);
			}
			v1 = ry * v1;
			v1 = rz * v1;
		}
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