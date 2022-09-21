#pragma once

#include "V3.hpp"
#include "M33.hpp"
#include "Geometry.hpp"
#include "framebuffer.h"
#include "gui.h"
#include "ppc.h"

#define FPS 1.0f
#define TIFF_FILE_IN "name.tif" // what we read from
#define TIFF_FILE_OUT "random.tif" // what we write to

class Scene {
public:
	GUI* gui;
	FrameBuffer* fb;
	PPC* ppc;

	int w, h;
	int frame;

	GEOMETRY geometry;

	Scene();
	void LoadTxtButton();
	void SaveTxtButton();
	void TransitionCamera();
};

extern Scene *scene;