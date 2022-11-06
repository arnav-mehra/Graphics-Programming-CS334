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

	PPC();
	
	bool project(V3 P, V3& new_p);
	V3 unproject(V3 pP);
	V3 directional_unproject(V3 pP);

	void pan(float alpha);
	void tilt(float alpha);
	void roll(float alpha);
	void rotate(M33& rot);
	void translateLR(float v);
	void translateUD(float v);
	void translateFB(float v);
	void zoom(float inc);

	void interpolate(PPC& cam1, PPC& cam2, float t);
	void interpolate_linear(PPC& cam1, PPC& cam2, float t);

	void reset();

	void LoadTxt();
	void SaveAsTxt();

	V3 GetVD();
	float GetPPu();
	float GetPPv();
};