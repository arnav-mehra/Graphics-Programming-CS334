#pragma once

#include "ppc.h"


PPC::PPC() {}

PPC::PPC(int _w, int _h) : w(_w), h(_h) {
	hfovd = DEG_TO_RAD(60.0f);

	C = V3(0.0f, 0.0f, 0.0f);

	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	c = V3(
		(float) -w * 0.5f,
		(float) h * 0.5f,
		(float) -w * 0.5f / tan(hfovd * 0.5f)
	);

	M = M33(a, b, c);
	cout << M;
	M.transpose();
	cout << M;
	M_inv = M.inverse();
	cout << M_inv;
}

void PPC::rotate(M33& rot) {
	a = rot * a;
	b = rot * b;
	c = rot * c;

	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

void PPC::reset() {
	hfovd = DEG_TO_RAD(60.0f);

	C = V3(0.0f, 0.0f, 0.0f);

	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	c = V3(
		(float)-w * 0.5f,
		(float)h * 0.5f,
		(float)-w * 0.5f / tan(hfovd * 0.5f)
	);

	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

void PPC::translateLR(float v) {
	V3 delta = a * v;
	C += delta;
}

void PPC::translateUD(float v) {
	V3 delta = b * v;
	C += delta;
}

void PPC::translateFB(float v) {
	V3 vd = GetVD();
	V3 delta = vd * v;
	C += delta;
}

void PPC::zoom(float hfovd_inc) {
	hfovd += DEG_TO_RAD(hfovd_inc);
	hfovd = max(DEG_TO_RAD(10.0f), hfovd);
	hfovd = min(DEG_TO_RAD(360.0f), hfovd);

	c = V3(
		(float)-w * 0.5f,
		(float)h * 0.5f,
		(float)-w * 0.5f / tan(hfovd * 0.5f)
	);

	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

bool PPC::Project(V3 P, V3& new_p) {
	V3 delta = P - C;
	V3 res = M_inv * delta;

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

// load a txt file
void PPC::LoadTxt() {
	ifstream in(INPUT_TXT);
	PPC copy;
	in.read((char*)&copy, sizeof(PPC));
	a = copy.a;
	b = copy.b;
	c = copy.c;
	C = copy.C;
	hfovd = copy.hfovd;
	// dont copy w & h, they are scene properties.	
	
	in.close();
}

// save as txt file
void PPC::SaveAsTxt() {
	ofstream out(OUTPUT_TXT);
	out.write((char*)this, sizeof(PPC));
	out.close();
}