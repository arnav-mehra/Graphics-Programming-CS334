#pragma once
#include "V3.hpp"
#include "M33.hpp"
#include "Geometry.hpp"

#include "framebuffer.hpp"
#include "gui.hpp"
#include "ppc.hpp"

#define FPS 10.0f
#define TIFF_FILE_IN "name.tif" // what we read from
#define TIFF_FILE_OUT "random.tif" // what we write to

class Scene {
public:
	GUI* gui;
	FrameBuffer* fb;
	PPC* ppc;

	int w, h;

	GEOMETRY geometry;

	V3 rotation_axis1;
	V3 rotation_axis2;

	Scene();
	void LoadTxtButton();
	void SaveTxtButton();
	void LoadBinButton();
	void SaveBinButton();
	void RotationButton();
	void TransitionCamera();
};

extern Scene *scene;