#pragma once

#include "gui.hpp"
#include "V3.hpp"
#include "framebuffer.hpp"
#include "Geometry.hpp"

#define FPS 10.0f
#define TIFF_FILE_IN "name.tif" // what we read from
#define TIFF_FILE_OUT "random.tif" // what we write to

class Scene {
public:
	GUI* gui;
	FrameBuffer* fb;
	PPC* ppc;

	U32 w, h;

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

	void LightLeft();
	void LightRight();
	void LightUp();
	void LightDown();
	void LightFront();
	void LightBack();
};

extern Scene *scene;