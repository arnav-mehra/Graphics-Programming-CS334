#include <iostream>
#include <fstream>
#include <chrono>

#include "Dimension.hpp"
#include "scene.h"

#include "_V3.hpp"
#include "_M33.hpp"

# define PI 3.14159265358979323846
#define DEG_TO_RAD(x) ((x) * (PI / 180.0f))

using namespace std;

Scene* scene;

Scene::Scene() {
	scene = this;

	h = 480;
	w = 640;
	frame = 0;
	gui = new GUI();
	gui->show();

	origin = V3((float) w * 0.5f, (float) h * 0.5f, 0.0f);
	perspective = M33(Dim::X, -0.1f) * M33(Dim::Y, 0.1f);

	// chosen shapes
	//POINT circle = POINT(
	//	V3(20.0f, 50.0f, 50.0f),
	//	COLOR(255, 0, 0)
	//);
	//SEGMENT line_seg = SEGMENT(
	//	V3(-100.0f, 0.0f, 0.0f),
	//	V3(100.0f, 100.0f, 0.0f),
	//	COLOR(0, 0, 255)
	//);
	//vector<V3> triangle = {
	//	V3(-100.0f, 0.0f, 100.0f),
	//	V3(100.0f, 0.0f, 100.0f),
	//	V3(0.0f, 100.0f, 100.0f)
	//};
	//vector<POINT> points = { circle };
	//vector<SEGMENT> lines = { line_seg };
	//vector<TRIANGLE> polys = { TRIANGLE(triangle, COLOR(0, 255, 0)) };

	V3 a_triangle_pnts[3] = {
		V3(0, 0, 0),
		V3(20, 50, 0),
		V3(40, 0, 0)
	};
	TRIANGLE a_triangle = TRIANGLE(a_triangle_pnts);
	SEGMENT a_line1 = SEGMENT(
		V3(0, 0, 0),
		V3(0, -50, 0),
		COLOR(255, 0, 0)
	);
	SEGMENT a_line2 = SEGMENT(
		V3(40, 0, 0),
		V3(40, -50, 0),
		COLOR(255, 0, 0)
	);
	GEOMETRY a = GEOMETRY({}, { a_line1, a_line2 }, { a_triangle });

	V3 r_triangle_vec[3] = {
		V3(60, 0, 0),
		V3(60, 50, 0),
		V3(100, 25, 0)
	};
	TRIANGLE r_triangle = TRIANGLE(r_triangle_vec);
	SEGMENT r_line1 = SEGMENT(
		V3(60, 0, 0),
		V3(60, -50, 0),
		COLOR(255, 0, 0)
	);
	SEGMENT r_line2 = SEGMENT(
		V3(60, 0, 0),
		V3(100, -50, 0),
		COLOR(255, 0, 0)
	);
	GEOMETRY r = GEOMETRY({}, { r_line1, r_line2 }, { r_triangle });

	SEGMENT n_line1 = SEGMENT(
		V3(120, -50, 0),
		V3(120, 50, 0),
		COLOR(255, 0, 0)
	);
	SEGMENT n_line2 = SEGMENT(
		V3(120, 50, 0),
		V3(160, -50, 0),
		COLOR(255, 0, 0)
	);
	SEGMENT n_line3 = SEGMENT(
		V3(160, -50, 0),
		V3(160, 50, 0),
		COLOR(255, 0, 0)
	);
	GEOMETRY n = GEOMETRY({}, { n_line1, n_line2, n_line3 }, {});

	vector<GEOMETRY> geos = { a, r, n };
	geometry = GEOMETRY();
	// geometry.add_axis();

	int u0 = 16, v0 = 40;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("SW framebuffer");
	fb->show();

	fb->SetBGR(0);
	fb->apply_geometry();
	fb->redraw();

	gui->uiw->position(u0+w+u0, v0);
}

void Scene::LoadTiffButton() {
	fb->LoadTiff("hi");
	fb->redraw();
}

void Scene::SaveTiffButton() {
	fb->SaveAsTiff("hi");
}

void Scene::TranslateImage() {
	//for (int i = 0; i < 1000; i++) {
		origin += V3(10, 0, 0);
		fb->SetBGR(0);
		fb->apply_geometry();
		fb->redraw();
	//}
}