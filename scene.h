#pragma once

#include "V3.hpp"
#include "M33.hpp"
#include "Geometry.hpp"
#include "framebuffer.h"
#include "gui.h"
#include "ppc.h"

#define FPS 30.0
#define TIFF_FILE_IN "name.tif" // what we read from
#define TIFF_FILE_OUT "random.tif" // what we write to

class Scene {
public:
	GUI* gui;
	FrameBuffer* fb;
	PPC* ppc;

	int w, h;
	int frame;
	float hfov;

	M33 perspective;
	V3 origin;
	GEOMETRY geometry;

	Scene();
	void LoadTiffButton();
	void SaveTiffButton();
	void TranslateImage();
};

extern Scene *scene;