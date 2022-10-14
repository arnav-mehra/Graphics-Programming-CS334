#include "M33.hpp"
#include "ppc.hpp"
#include "scene.hpp"

PPC::PPC() {
	reset();
}

void PPC::pan(float alpha) {
	M33 rot = M33::get_rotation_matrix(C, C - b, alpha);
	rotate(rot);
}

void PPC::tilt(float alpha) {
	M33 rot = M33::get_rotation_matrix(C, C + a, alpha);
	rotate(rot);
}

void PPC::roll(float alpha) {
	M33 rot = M33::get_rotation_matrix(C, C + (a ^ b), alpha);
	rotate(rot);
}

void PPC::rotate(M33& rot) {
	a *= rot;
	b *= rot;
	c *= rot;
	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

void PPC::reset() {
	float hfovd = DEG_TO_RAD(60.0f);

	C = V3(0.0f, 0.0f, 0.0f);

	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	c = V3(
		-(float)scene->w * 0.5f,
		(float)scene->h * 0.5f,
		-(float)scene->w * 0.5f / tan(hfovd * 0.5f)
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
	// float hfovd = 2 * atan(scene->w / (2.0f * a.length() / f));
	c = a * (-GetPPu()) - b * GetPPv() + vd * f;

	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

bool PPC::project(V3 P, V3& new_p) {
	V3 res = M_inv * (P - C);

	new_p = res;
	if (new_p[Dim::Z] <= 0.0f) {
		return false;
	}
	new_p[Dim::Z] = 1.0f / new_p[Dim::Z];
	new_p[Dim::X] *= new_p[Dim::Z];
	new_p[Dim::Y] *= new_p[Dim::Z];
	return true;
}

V3 PPC::unproject(V3 pP) {
	return C + (a * pP[Dim::X] + b * pP[Dim::Y] + c) / pP[Dim::Z];
}

void PPC::interpolate(PPC& cam1, PPC& cam2, float t) {
	C = cam2.C * t + cam1.C * (1.0f - t);

	// magnitudes are still linear
	float c_mag = cam2.c.length() * t + cam1.c.length() * (1.0f - t);
	float b_mag = cam2.b.length() * t + cam1.b.length() * (1.0f - t);
	float a_mag = cam2.a.length() * t + cam1.a.length() * (1.0f - t);
	// vector angles made linear (actual vector is not linear
	float c_cos_theta = (cam1.c * cam2.c) / sqrt((cam1.c * cam1.c) * (cam2.c * cam2.c));
	float c_theta = acos(c_cos_theta);
	V3 c_axis = cam1.c ^ cam2.c;
	c = cam1.c;
	c.rotate(c_axis, c_theta * t);
	c.normalize();
	c *= c_mag;

	float b_cos_theta = (cam1.b * cam2.b) / sqrt((cam1.b * cam1.b) * (cam2.b * cam2.b));
	float b_theta = acos(b_cos_theta);
	V3 b_axis = cam1.b ^ cam2.b;
	b = cam1.b;
	b.rotate(b_axis, b_theta * t);
	b.normalize();
	b *= b_mag;

	float a_cos_theta = (cam1.a * cam2.a) / sqrt((cam1.a * cam1.a) * (cam2.a * cam2.a));
	float a_theta = acos(a_cos_theta);
	V3 a_axis = cam1.a ^ cam2.a;
	a = cam1.a;
	a.rotate(a_axis, a_theta * t);
	a.normalize();
	a *= a_mag;

	M = M33(a, b, c);
	M.transpose();
	M_inv = M.inverse();
}

void PPC::interpolate_linear(PPC& cam1, PPC& cam2, float t) {
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

// load a txt file
void PPC::LoadTxt() {
	ifstream in(INPUT_TXT);
	in.read((char*) this, sizeof(PPC));
	in.close();
}

// save as txt file
void PPC::SaveAsTxt() {
	ofstream out(OUTPUT_TXT);
	cout << a << b << c << C;
	out.write((char*)this, sizeof(PPC));
	out.close();
}