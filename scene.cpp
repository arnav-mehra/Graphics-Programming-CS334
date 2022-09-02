#include <iostream>
#include <fstream>
#include <chrono>

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
	frame = 0;
	gui = new GUI();
	gui->show();

	origin = V3((float)w * 0.5f, (float)h * 0.5f, 0.0f);
	perspective = M33(0); // M33(Dim::X, -0.1f)* M33(Dim::Y, 0.1f);

	// chosen points (note that last vector is color)
	POINT3 p3 = POINT3(
		V3(20.0f, 50.0f, 50.0f),
		V3(255, 0, 0)
	);
	LINE3 axis = LINE3(
		V3(-100.0f, 0.0f, 200.0f),
		V3(100.0f, 100.0f, -100.0f),
		V3(0, 0, 255)
	);
	vector<POINT3> points = { p3 };
	vector<LINE3> lines = { axis };

	// GRAPH TESTING
	

	geometry = GEOMETRY(points, lines);
	geometry.add_axis();

	int u0 = 16, v0 = 40;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("SW framebuffer");
	fb->show();
	fb->SetBGR(0);
	fb->redraw();
	gui->uiw->position(u0+w+u0, v0);
}

void Scene::DBG() {
	perspective = M33(Dim::Y, 0.05f) * perspective;
	fb->recompute_geometry();
	fb->SetBGR(0);
	fb->redraw();
	cerr << "INFO: ROTATED PERSPECTIVE" << endl;
}

void Scene::NewButton() {
	if (!geometry.points.size() || !geometry.lines.size()) {
		cerr << "UNABLE TO ROTATE POINT, CODE IS LIKELY COMMENTED" << endl;
	}

	V3& p = geometry.points[0].point;
	LINE3& axis = geometry.lines[0];
		
	float thetha = frame * (3.14f / 180.0f);
	p.rotate(axis.start, axis.end, 2 * .0174);
	cerr << p;
	float x = frame * 1.0f;
	geometry.points.push_back(
		POINT3(V3(x, p[Dim::X], 0), V3(255, 0, 0))
	);
	geometry.points.push_back(
		POINT3(V3(x, p[Dim::Y], 0), V3(0, 255, 0))
	);
	geometry.points.push_back(
		POINT3(V3(x, p[Dim::Z], 0), V3(0, 0, 255))
	);
	frame++;

	fb->recompute_geometry();
	fb->SetBGR(0);
	fb->redraw();
	cerr << "INFO: pressed New button on GUI" << endl;
}