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
	float hfovd;

	PPC();
	
	bool Project(V3 P, V3& new_p);

	void rotate(M33& rot);
	void translateLR(float v);
	void translateUD(float v);
	void translateFB(float v);
	void zoom(float _hfovd);

	void interpolate(PPC& cam1, PPC& cam2, float t);

	void reset();

	void LoadTxt();
	void SaveAsTxt();

	V3 GetVD();
	float GetPPu();
	float GetPPv();
	void updateHfov();
};