#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "V3.hpp"
#include "Geometry.hpp"

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix; // pixel array
	int w, h;
	V3 *xyz;
	PRECOMPUTE_GEOMETRY precompute;

	FrameBuffer(int u0, int v0, int _w, int _h);
	
	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	void apply_geometry();

	void FrameBuffer::LoadTiff(char* fname);
	void FrameBuffer::SaveAsTiff(char* fname);
};