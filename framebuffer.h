#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>
#include <thread>

#include "V3.hpp"
#include "Geometry.hpp"

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix; // pixel array
	int w, h;
	V3 *xyz;
	COMPUTED_GEOMETRY compute;
	thread tr;

	FrameBuffer(int u0, int v0, int _w, int _h);
	
	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	void applyGeometry();
	// void nextFrame(void* window);
	void startThread();

	void FrameBuffer::LoadTiff();
	void FrameBuffer::SaveAsTiff();
};