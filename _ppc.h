#pragma once
#include "ppc.h"
#include "M33.hpp"

PPC::PPC(float hfov, int _w, int _h) : w(_w), h(_h) {
	C = V3(320.0f, 240.0f, 0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);

	float hfovd = DEG_TO_RAD(hfov);
	c = V3(
		(float) -w * 0.5f,
		(float) h * 0.5f,
		(float) -w * 0.5f / tan(hfovd * 0.5f)
	);
	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

bool PPC::Project(V3 P, V3& new_p) {
	//cout << M_inv;
	//cout << P;
	//cout << C;

	V3 delta = P - C;
	V3 res = M_inv * delta;

	//cout << delta;
	//cout << res;

	new_p = res;
	if (new_p[Dim::Z] <= 0.0f) {
		return false;
	}
	float z_inv = 1.0f / new_p[Dim::Z];
	new_p[Dim::X] *= z_inv;
	new_p[Dim::Y] *= z_inv;
	return true;
}

V3 PPC::GetVD() {
	V3 res = a ^ b;
	res.normalize();
	return res;
}