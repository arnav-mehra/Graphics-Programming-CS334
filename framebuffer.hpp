#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "ppc.hpp"
#include "Geometry.hpp"

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix; // pixel array
	U32 w, h;
	COMPUTED_GEOMETRY compute;

	PPC cam1;
	PPC cam2;
	PPC cam3;
	float transition1;
	float transition2;

	FrameBuffer(int u0, int v0, U32 _w, U32 _h);

	void draw();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	
	void applyGeometry();
	inline void applySphere(SPHERE& sphere, vector<float>& z_index);
	inline void applySegment(SEGMENT& seg, vector<float>& z_index);
	inline void applyTriangle(TRIANGLE& tri, vector<float>& z_index);

	void startThread();

	void LoadTiff();
	void SaveAsTiff();
};