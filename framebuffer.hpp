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
	PPC cam4;
	float transition1;
	float transition2;
	float transition3;
	U32 frame = 0;

	vector<float> z_buffer; // z-buffer, make w & h defines -> stack
	vector<int> tri_buffer;

	FrameBuffer(int u0, int v0, U32 _w, U32 _h);

	void draw();
	void startThread();
	void KeyboardHandle();
	int handle(int guievent);
	void SetBGR(unsigned int bgr);
	
	void applyGeometry();
	inline void applySphere(SPHERE& sphere);
	inline void applySegment(U32 i);
	inline void applyTriangle(U32 i);
	void applyTriangleLight(U32 t_i, U32 l_i);
	inline void applyLights();
	inline void applySM2Light(U32 i);

	void LoadTiff();
	void SaveAsTiff();
	void SaveAsTiff(const char* fname);
};