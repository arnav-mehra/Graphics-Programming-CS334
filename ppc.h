#pragma once

#include "V3.hpp"

class PPC {
public:
	V3 a, b, c, C;
	M33 M, M_inv;
	int w, h;
	PPC(float hfov, int _w, int _h);
	bool PPC::Project(V3 P, V3& new_p);
	V3 GetVD();
};