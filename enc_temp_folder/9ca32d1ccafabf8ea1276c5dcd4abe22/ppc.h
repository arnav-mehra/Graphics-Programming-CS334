#pragma once
#include <fstream>
#include "V3.hpp"
#include "M33.hpp"

#define INPUT_TXT "out.txt"
#define OUTPUT_TXT "out.txt"

class PPC {
public:
	V3 a, b, c, C;
	M33 M, M_inv;
	int w, h;
	float hfovd;

	PPC();
	PPC(int _w, int _h);
	
	bool Project(V3 P, V3& new_p);

	void rotate(M33& rot);
	void translateLR(float v);
	void translateUD(float v);
	void translateFB(float v);
	void zoom(float _hfovd);

	void interpolate(PPC& cam1, PPC& cam2, float t) {
		a = cam1.a * t + cam2.a * (1.0f - t);
		b = cam1.b * t + cam2.b * (1.0f - t);
		c = cam1.c * t + cam2.c * (1.0f - t);
		C = cam1.C * t + cam2.C * (1.0f - t);
		hfovd = cam1.hfovd * t + cam2.hfovd * (1.0f - t);
	}

	void reset();

	void LoadTxt();
	void SaveAsTxt();

	V3 GetVD();
};