#pragma once

#include "V3.hpp"
#include "M33.hpp"
#include "Geometry.hpp"
#include "framebuffer.h"
#include "gui.h"

class Scene {
public:
	GUI *gui;
	FrameBuffer *fb;

	int w, h;
	int frame;

	M33 perspective;
	V3 origin;
	GEOMETRY geometry;

	Scene();
	void RotatePerspective();
	void RotatePoint();
};

extern Scene *scene;