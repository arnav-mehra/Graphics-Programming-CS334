#pragma once

#include "V3.hpp"
#include "M33.hpp"
#include "Geometry.hpp"
#include "framebuffer.h"
#include "gui.h"

#define PLAY_PONG false
#define PLAY_NAME_SCROLL true
#define SHOW_GEOMETRY false
#define TIFF_FILE_IN "name.tif" // what we read from
#define TIFF_FILE_OUT "random.tif" // what we write to

class Scene {
public:
	GUI *gui;
	FrameBuffer *fb;

	int w, h;
	int frame;

	M33 perspective;
	V3 origin;
	GEOMETRY geometry;
	V3 player1;
	V3 player2;
	V3 ball_pos;
	V3 ball_vel;

	int s1 = 0, s2 = 0;

	Scene();
	void LoadTiffButton();
	void SaveTiffButton();
	void TranslateImage();
};

extern Scene *scene;