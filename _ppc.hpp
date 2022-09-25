#pragma once

#include "ppc.hpp"

PPC::PPC() {
	reset();
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
		(float)-scene->w * 0.5f,
		(float) scene->h * 0.5f,
		(float)-scene->w * 0.5f / tan(hfovd * 0.5f)
	);

	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

void PPC::translateLR(float v) {
	V3 a_cpy = a;
	a_cpy.normalize();
	V3 delta = a_cpy * v;
	C += delta;
}

void PPC::translateUD(float v) {
	V3 b_cpy = b;
	b_cpy.normalize();
	V3 delta = b_cpy * v;
	C += delta;
}

void PPC::translateFB(float v) {
	V3 vd = GetVD();
	V3 delta = vd * v;
	C += delta;
}

void PPC::zoom(float inc) {
	V3 vd = GetVD();

	float f = vd * c;
	f *= inc;
	hfovd = 2 * atan(scene->w / (2.0f * a.length() / f));	
	c = a * (-GetPPu()) - b * GetPPv() + vd * f;

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

void PPC::interpolate(PPC& cam1, PPC& cam2, float t) {
	t *= t;
	C = cam2.C * t + cam1.C * (1.0f - t);
	a = cam2.a * t + cam1.a * (1.0f - t);
	b = cam2.b * t + cam1.b * (1.0f - t);
	c = cam2.c * t + cam1.c * (1.0f - t);
	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

V3 PPC::GetVD() {
	V3 res = a ^ b;
	res.normalize();
	return res;
}

float PPC::GetPPu() {
	V3 a_c = a;
	a_c.normalize();
	float val = c * a_c / a.length();
	return val * -1.0f;
}

float PPC::GetPPv() {
	V3 b_c = b;
	b_c.normalize();
	float val = c * b_c / b.length();
	return val * -1.0f;
}

void PPC::updateHfov() {
	
}

// load a txt file
void PPC::LoadTxt() {
	ifstream in(INPUT_TXT);
	in.read((char*) this, sizeof(PPC));
	in.close();
}

// save as txt file
void PPC::SaveAsTxt() {
	ofstream out(OUTPUT_TXT);
	out.write((char*)this, sizeof(PPC));
	out.close();
}