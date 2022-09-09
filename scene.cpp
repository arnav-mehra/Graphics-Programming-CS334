#include "Dimension.hpp"
#include "scene.h"

#include "_V3.hpp"
#include "_M33.hpp"

#define PI 3.14159265358979323846
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
	perspective = M33(Dim::X, 0); // M33(Dim::X, -0.1f)* M33(Dim::Y, 0.1f);

	player1 = V3(-200, 0, 0);
	player2 = V3(-200, 0, 0);
	ball_pos = V3(0, 0, 0);
	ball_vel = V3(3, 2, 0);

	// GEOMETRY SHOWCASE
	if (SHOW_GEOMETRY) {
		SPHERE circle = SPHERE(
			V3(-100.0f, 60.0f, -100.0f),
			COLOR(0, 255, 0),
			60
		);
		SEGMENT line_seg = SEGMENT(
			V3(-100.0f, 50.0f, 0.0f),
			V3(100.0f, 100.0f, 0.0f),
			COLOR(0, 0, 255)
		);
		V3 triangle_pts[] = {
			V3(-100.0f, 0.0f, 100.0f),
			V3(100.0f, 0.0f, 100.0f),
			V3(0.0f, 100.0f, 100.0f)
		};
		TRIANGLE triangle = TRIANGLE(triangle_pts);
		geometry = GEOMETRY({ circle }, { line_seg }, { triangle_pts });
	}

	// NAME SCROLL
	if (PLAY_NAME_SCROLL) {
		origin = V3(0.0f, (float)h * 0.5f, 0.0f);

		V3 a1[3] = {
			V3(40, 50, 0),
			V3(80, -50, 0),
			V3(60, -50, 0)
		};
		geometry.add_triangle(TRIANGLE(a1));
		V3 a2[3] = {
			V3(40, 50, 0),
			V3(20, -20, 0),
			V3(60, -20, 0)
		};
		geometry.add_triangle(TRIANGLE(a2));
		V3 a3[3] = {
			V3(40, 50, 0),
			V3(0, -50, 0),
			V3(20, -50, 0)
		};
		geometry.add_triangle(TRIANGLE(a3));
		geometry.add_sphere(SPHERE(V3(40, 0, -10), COLOR(255, 255, 255), 20));

		V3 r1[3] = {
			V3(100, 0, 0),
			V3(140, -50, 0),
			V3(100, 50, 0)
		};
		geometry.add_triangle(TRIANGLE(r1));
		V3 r2[3] = {
			V3(100, 50, 0),
			V3(150, 25, 0),
			V3(100, -50, 0)
		};
		geometry.add_triangle(TRIANGLE(r2));
		geometry.add_sphere(SPHERE(V3(120, 20, -10), RGB(255, 255, 255), 20));

		V3 n_triangle_vec1[3] = {
			V3(170, 50, 0),
			V3(190, 50, 0),
			V3(170, -50, 0)
		};
		geometry.add_triangle(TRIANGLE(n_triangle_vec1));
		V3 n_triangle_vec2[3] = {
			V3(230, 50, 0),
			V3(230, -50, 0),
			V3(210, -50, 0)
		};
		geometry.add_triangle(TRIANGLE(n_triangle_vec2));
		V3 n_triangle_vec4[3] = {
			V3(170, 50, 0),
			V3(190, 50, 0),
			V3(210, -50, 0)
		};
		geometry.add_triangle(TRIANGLE(n_triangle_vec4));
		V3 n_triangle_vec3[3] = {
			V3(190, 50, 0),
			V3(230, -50, 0),
			V3(210, -50, 0)
		};
		geometry.add_triangle(TRIANGLE(n_triangle_vec3));
	}

	// EXTRA CREDIT: PONG
	if (PLAY_PONG) {
		geometry.setup_pong();
	}

	if (PLAY_TETRIS) {
		origin = V3(120.0f, 50, 0.0f);
		geometry.setup_tetris();
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