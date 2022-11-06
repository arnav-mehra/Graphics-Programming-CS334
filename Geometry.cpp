#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "M33.hpp"
#include "Geometry.hpp"
#include "scene.hpp"
#include "ppc.hpp"

COLOR::COLOR() {}

COLOR::COLOR(U32 v) {
	value = v;
}

COLOR::COLOR(V3 col) {
	U32 r = min((U32)col[Dim::X], 255U);
	U32 g = min((U32)col[Dim::Y], 255U);
	U32 b = min((U32)col[Dim::Z], 255U);
	value = (b << 16) | (g << 8) | r;
}

COLOR::COLOR(U32 r, U32 g, U32 b) {
	r = min(r, 255U);
	g = min(g, 255U);
	b = min(b, 255U);
	value = (b << 16) | (g << 8) | r;
}

U32 COLOR::getR() const {
	return (value) & 255;
}

U32 COLOR::getG() const {
	return (value >> 8) & 255;
}

U32 COLOR::getB() const {
	return (value >> 16) & 255;
}

COLOR COLOR::operator*(const float scalar) {
	U32 r = (U32) (getR() * scalar);
	U32 g = (U32) (getG() * scalar);
	U32 b = (U32) (getB() * scalar);
	return COLOR(r, g, b);
}

void COLOR::operator*=(const float scalar) {
	(*this) = (*this) * scalar;
}

COLOR COLOR::operator+(const COLOR& color) {
	return COLOR(value + color.value);
}

COLOR COLOR::interpolate(COLOR& color, float v) {
	return (*this) * (1.0f - v) + (color) * (v);
}

ostream& operator<<(ostream& out, COLOR& c) {
	out << '(' << c.getR() << ", " << c.getG() << ", " << c.getB() << ')';
	return out;
}

COLOR INTERPOLATE::getColor(SEGMENT& seg, V3& pos) {
	float d1 = 1.0f / (pos - seg.start.point).length();
	float d2 = 1.0f / (pos - seg.end.point).length();
	float d_ = 1.0f / (d1 + d2);
	COLOR res = seg.start.color * (d_ * d1) + seg.end.color * (d_ * d2);
	return res;
}

COLOR INTERPOLATE::getColor(TRIANGLE& tri, V3& pos) {
	float d1 = 1.0f / (pos - tri.points[0]).length();
	float d2 = 1.0f / (pos - tri.points[1]).length();
	float d3 = 1.0f / (pos - tri.points[2]).length();
	float d_ = 1.0f / (d1 + d2 + d3);
	COLOR res = tri.colors[0] * (d_ * d1) +
				tri.colors[1] * (d_ * d2) +
				tri.colors[2] * (d_ * d3);
	return res;
}

COLOR INTERPOLATE::getColor(TRIANGLE& tri, COLOR& c1, COLOR& c2, COLOR& c3, V3& pos) {
	float d1 = 1.0f / (pos - tri.points[0]).length();
	float d2 = 1.0f / (pos - tri.points[1]).length();
	float d3 = 1.0f / (pos - tri.points[2]).length();
	float d_ = 1.0f / (d1 + d2 + d3);
	COLOR res = c1 * (d_ * d1) + c2 * (d_ * d2) + c3 * (d_ * d3);
	return res;
}

TEXTURE::TEXTURE() {}

TEXTURE::TEXTURE(COLOR c) {
	w = 1;
	h = 1;
	texture.resize(h, vector<COLOR>(w, c));
}

TEXTURE::TEXTURE(char* fName) {
	TIFF* in = TIFFOpen(fName, "r");
	if (in == NULL) {
		cout << fName << " could not be opened\n";
		return;
	}

	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &h);
	texture.resize(h, vector<COLOR>(w, 0));

	U32* temp = new U32[w * h];
	if (!TIFFReadRGBAImage(in, w, h, temp, 0)) {
		cout << "failed to load\n";
		return;
	}

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			texture[i][j] = COLOR(temp[i * w + j]);
		}
	}

	cout << "Loaded texture with size: " << w << " x " << h << '\n';
	TIFFClose(in);
}

void TEXTURE::transform(bool transposed, bool x_mirror, bool y_mirror) {
	vector<vector<COLOR>> newTx;
	if (transposed) {
		newTx.resize(w, vector<COLOR>(h));
	} else {
		newTx.resize(h, vector<COLOR>(w));
	}
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			int x = x_mirror ? w - 1 - i : i;
			int y = y_mirror ? h - 1 - j : j;
			COLOR c = texture[y][x];
			if (transposed) {
				newTx[i][j] = texture[y][x];
			} else {
				newTx[j][i] = texture[y][x];
			}
		}
	}
	texture = newTx;
	h = newTx.size();
	w = h == 0 ? 0 : newTx[0].size();
}

TEXTURE_META::TEXTURE_META() {}

TEXTURE_META::TEXTURE_META(V3 (&pts)[3], TEXTURE* tx, int tf) {
	V3 a = pts[0];
	V3 b = pts[1];
	V3 c = pts[2];
	t[Dim::X] = a - b;
	t[Dim::Y] = c - b;
	t[Dim::Z] = b;
	t.transpose();
	t = t.inverse();

	V3 t1 = V3(tx->w - 1, 0.0f, 0.0f);
	V3 t2 = V3(0.0f, tx->h - 1, 0.0f);
	M33 mult = M33(t1, t2, V3(0.0f, 0.0f, 1.0f));
	mult.transpose();
	t = mult * t;

	this->tx = tx;
	tile_factor = tf;
	mirroring = false;
}

COLOR TEXTURE_META::proj(V3 p) {
	V3 coord = t * p;
	float xf = coord[Dim::X] * tile_factor;
	float yf = coord[Dim::Y] * tile_factor;

	bool x_mirrored = ((int)round(xf) / tx->w) % 2;
	bool y_mirrored = ((int)round(yf) / tx->h) % 2;

	xf = fmodf(xf, (float)tx->w);
	yf = fmodf(yf, (float)tx->h);
	if (mirroring) {
		if (x_mirrored) xf = (float)tx->w - 1.0f - xf;
		if (y_mirrored) yf = (float)tx->h - 1.0f - yf;
	}

	int x1 = (int)floorf(xf);
	int y1 = (int)floorf(yf);
	int x2 = (int)ceilf(xf);
	int y2 = (int)ceilf(yf);
	float dx = xf - (float)x1;
	float dy = yf - (float)y1;

	return tx->get(x1, y1) * (1.0f - dx) * (1.0f - dy)
		 + tx->get(x1, y2) * (1.0f - dx) * dy
		 + tx->get(x2, y1) * dx * (1.0f - dy)
		 + tx->get(x2, y2) * dx * dy;
}

SPHERE::SPHERE() {}

SPHERE::SPHERE(V3 point) {
	this->point = point;
	this->width = 10;
	this->color = COLOR(255, 0, 0);
}

SPHERE::SPHERE(V3 point, COLOR color) : SPHERE(point) {
	this->color = color;
	this->width = 10;
}

SPHERE::SPHERE(V3 point, COLOR color, U32 width) : SPHERE(point, color) {
	this->width = width;
}

SEGMENT::SEGMENT() {}

SEGMENT::SEGMENT(SPHERE start, SPHERE end) {
	this->start = start;
	this->end = end;
	this->width = 5;
}

SEGMENT::SEGMENT(SPHERE start, SPHERE end, U32 width) : SEGMENT(start, end) {
	this->width = width;
}

TRIANGLE::TRIANGLE() {
	phong_exp = 1.0f;
	width = 8U;
}

TRIANGLE::TRIANGLE(vector<V3> points) : TRIANGLE() {
	this->points[0] = points[0];
	this->points[1] = points[1];
	this->points[2] = points[2];
	norm = (points[1] - points[0]) ^ (points[2] - points[0]);
	norm.normalize();
	tm = nullptr;
}

TRIANGLE::TRIANGLE(vector<V3> points, TEXTURE_META* tm) : TRIANGLE(points) {
	this->tm = tm;
}

MESH::MESH() {
	fill = false;
}

MESH::MESH(vector<TRIANGLE> tris) : MESH() {
	for (TRIANGLE& tri : tris) {
		add_triangle(tri);
	}
}

MESH::MESH(vector<TRIANGLE> tris, bool fill) : MESH(tris) {
	this->fill = fill;
}

MESH::MESH(vector<TRIANGLE> tris, bool fill, float phong_exp) : MESH(tris, fill) {
	for (TRIANGLE& tri : tris) {
		tri.phong_exp = phong_exp;
	}
}

void MESH::add_tiling(float inc) {
	for (int i = 0; i < triangles.size(); i += 2) {
		triangles[i].tm->tile_factor += inc;
	}
}

void MESH::add_mirroring(bool m) {
	for (int i = 0; i < triangles.size(); i += 2) {
		triangles[i].tm->mirroring = m;
	}
}

void MESH::fix_normals() {
	V3 center = get_center();
	for (int i = 0; i < triangles.size(); i++) {
		TRIANGLE& tri = triangles[i];
		V3 tri_center = (tri.points[0] + tri.points[1] + tri.points[2]);
		tri_center /= 3.0f;
		V3 delta = tri_center - center;
		if (delta * tri.norm < 0.0f) // angle is obtuse
			tri.norm *= -1.0f;
	}
}

void MESH::set_phong_exp(float exp) {
	for (int i = 0; i < triangles.size(); i++) {
		triangles[i].phong_exp = exp;
	}
}

V3 MESH::get_center() {
	V3 sum = V3(0.0f, 0.0f, 0.0f);
	if (triangles.size() == 0) return sum;
	for (int i = 0; i < triangles.size(); i++) {
		for (V3& v : triangles[i].points) {
			sum += v;
		}
	}
	V3 res = sum / ((float) triangles.size() * 3.0f);
	return res;
}

void MESH::translate(V3 tran) {
	for (int i = 0; i < triangles.size(); i++) {
		for (V3& v : triangles[i].points) {
			v += tran;
		}
	}
}

void MESH::position(V3 pos) {
	V3 center = get_center();
	translate(pos - center);
}

void MESH::scale(float s) {
	V3 center = get_center();
	for (int i = 0; i < triangles.size(); i++) {
		for (V3& v : triangles[i].points) {
			V3 delta = v - center;
			V3 scaled = delta * s;
			v = scaled + center;
		}
	}
}

void MESH::rotate(V3 axis1, V3 axis2, float alpha) {
	M33 total_rotation = M33::get_rotation_matrix(axis1, axis2, alpha);
	for (int i = 0; i < triangles.size(); i++) {
		for (V3& v : triangles[i].points) {
			v -= axis1;
			v = total_rotation * v;
			v += axis1;
		}
	}
}

float MESH::avg_dist_from_center() {
	V3 c = get_center();
	float total_dist = 0.0f;
	if (triangles.size() == 0) return total_dist;
	for (int i = 0; i < triangles.size(); i++) {
		for (V3& v : triangles[i].points) {
			V3 delta = v - c;
			total_dist += delta.length();
		}
	}
	return total_dist / (float) (triangles.size() * 3);
}

void MESH::spherical_interpolation(float t, float res) {
	// break down big triangles
	for (int i = 0; i < triangles.size(); i++) {
		V3& s1 = triangles[i].points[0];
		V3& s2 = triangles[i].points[1];
		V3& s3 = triangles[i].points[2];
		float d1 = (s1 - s2).length();
		float d2 = (s1 - s3).length();
		float d3 = (s2 - s3).length();
		float mx = max3(d1, d2, d3);
		if (mx < res) continue;
		if (mx == d1) {
			V3 inter = (s1 + s2) * 0.5f;
			add_triangle(TRIANGLE({ s1, inter, s3 }));
			s1 = inter; // inter, 2, 3
		}
		else if (mx == d2) {
			V3 inter = (s1 + s3) * 0.5f;
			add_triangle(TRIANGLE({ s1, inter, s2 }));
			s1 = inter; // inter, 2, 3
		}
		else {
			V3 inter = (s2 + s3) * 0.5f;
			add_triangle(TRIANGLE({ s1, inter, s3 }));
			s3 = inter; // 1, 2, inter
		}
	}
	fix_normals();
	// round triangles.
	float avg_dist = avg_dist_from_center();
	V3 c = get_center();
	for (int i = 0; i < triangles.size(); i++) {
		for (V3& v : triangles[i].points) {
			V3 delta = v - c;
			float len = delta.length();
			float dist = (len - avg_dist) * t;
			delta /= len;
			v -= delta * dist;
		}
	}
}

void MESH::setAsBox(V3 center, TEXTURE* tx, float w, float h, float l) {
	triangles.clear();

	auto A = V3(0, 0, 0);
	auto B = V3(w, 0, 0);
	auto C = V3(w, h, 0);
	auto D = V3(0, h, 0);

	auto E = V3(0, 0, l);
	auto F = V3(w, 0, l);
	auto G = V3(w, h, l);
	auto H = V3(0, h, l);

	*this = MESH({
		// ABCD
		TRIANGLE({ A, B, C }),
		TRIANGLE({ A, D, C }),
		// EFGH
		TRIANGLE({ E, F, G }),
		TRIANGLE({ E, H, G }),
		// CDHG
		TRIANGLE({ C, D, H }),
		TRIANGLE({ C, G, H }),
		// ABFE
		TRIANGLE({ A, B, F }),
		TRIANGLE({ A, E, F }),
		// DAEH
		TRIANGLE({ D, A, E }),
		TRIANGLE({ D, H, E }),
		// CBFG
		TRIANGLE({ C, B, F }),
		TRIANGLE({ C, G, F })
	});
	position(center);

	for (int i = 0; i < this->triangles.size(); i += 2) {
		TRIANGLE& t1 = this->triangles[i];
		TRIANGLE& t2 = this->triangles[i + 1];
		t1.tm = new TEXTURE_META(t1.points, tx, 1);
		t2.tm = t1.tm;
	}
}

void MESH::setAsSphere(V3 center, U32 hres, float radius, TEXTURE* tx) {
	triangles.clear();
	hres = min(hres, 20U);

	V3 vec = V3(0.0f, 0.0f, radius);
	U32 vres = hres;
	float alpha = 2.0f * (float) PI / (float) hres;
	M33 rot1 = M33(Dim::Y, alpha);
	M33 rot2 = M33(Dim::X, -alpha);
	M33 rot(1);

	vector<vector<V3>> vecs(vres, vector<V3>(hres));
	for (U32 theta = 0; theta < vres; theta++) {
		for (U32 phi = 0; phi < hres; phi++) {
			vecs[theta][phi] = vec;
			vec = rot1 * vec;
		}
		vec = rot2 * vec;
	}

	for (U32 a = 0; a < vres; a++) {
		for (U32 b = 0; b < hres / 2; b++) {
			U32 i = (a + 1U) % hres;
			U32 j = (b + 1U) % hres;
			V3 v1 = vecs[a][b];
			v1 += center;
			V3 v2 = vecs[i][b];
			v2 += center;
			V3 v3 = vecs[a][j];
			v3 += center;
			V3 v4 = vecs[i][j];
			v4 += center;
			TEXTURE_META* tm = new TEXTURE_META();
			TRIANGLE t1 = TRIANGLE({ v1, v2, v4 }, tm);
			*tm = TEXTURE_META(t1.points, tx, 1);
			add_triangle(t1);
			add_triangle(TRIANGLE({ v1, v3, v4 }, tm));
		}
	}
}

void MESH::setAsCylinder(V3 center, U32 res, float height, float rad) {
	triangles.clear();
	res = min(res, 200U);

	float rad_inc = rad / (float) res;
	float alpha = 2.0f * (float)PI / (float)res;
	M33 rot = M33(Dim::Y, alpha);

	vector<V3> upper_vecs(res);
	vector<V3> lower_vecs(res);
	V3 vec = V3(rad, 0.0, 0.0);
	for (U32 theta = 0; theta < res; theta++) {
		V3 upper = vec;
		V3 lower = vec;
		upper[Dim::Y] += height * 0.5f;
		lower[Dim::Y] -= height * 0.5f;
		upper_vecs[theta] = upper;
		lower_vecs[theta] = lower;
		vec = rot * vec;
	}

	V3 upper_center = V3(0.0f, height * 0.5f, 0.0f);
	V3 lower_center = V3(0.0f, height * -0.5f, 0.0f);
	for (U32 a = 0; a < res; a++) {
		U32 i = (a + 1U) % res;
		V3& v1 = lower_vecs[a];
		V3& v2 = lower_vecs[i];
		V3& v3 = upper_vecs[a];
		V3& v4 = upper_vecs[i];
		add_triangle(TRIANGLE({ v1, v2, v4 }));
		add_triangle(TRIANGLE({ v1, v3, v4 }));
		add_triangle(TRIANGLE({ v3, v4, upper_center }));
		add_triangle(TRIANGLE({ v1, v2, lower_center }));
	}
	position(center);
}

void MESH::setAsFloor(TEXTURE* tx) {
	triangles.clear();
	float y = -50.0f;
	for (int x = -500.0f; x <= 500.0f; x += 50.0f) {
		for (int z = -500.0f; z <= 500.0f; z += 50.0f) {
			TEXTURE_META* tm = new TEXTURE_META();
			TRIANGLE t1 = TRIANGLE({
				V3(x, y, z),
				V3(x + 50.0f, y, z),
				V3(x + 50.0f, y, z + 50.0f)
				}, tm);
			*tm = TEXTURE_META(t1.points, tx, 1);
			TRIANGLE t2 = TRIANGLE({
				V3(x + 50.0f, y, z + 50.0f),
				V3(x, y, z + 50.0f),
				V3(x, y, z)
			}, tm);
			this->add_triangle(t1);
			this->add_triangle(t2);
		}
	}
	fill = true;
}

void MESH::LoadBin() {
	cout << "Loading Mesh Bin...\n";
	ifstream in(INPUT_BIN);
	in.read((char*)this, sizeof(MESH));
	in.close();
}

void MESH::Load334Bin() {
	ifstream ifs("teapot1K.bin", ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << endl;
		return;
	}

	int vertsN;
	ifs.read((char*)&vertsN, sizeof(int));

	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}

	V3* verts = 0;
	V3* cols = 0;
	V3* normals = 0;
	U32* tris = 0;

	verts = new V3[vertsN];
	ifs.read(&yn, 1); // cols 3 floats
	if (yn == 'y') cols = new V3[vertsN];

	ifs.read(&yn, 1); // normals 3 floats
	normals = 0;
	if (yn == 'y') normals = new V3[vertsN];

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float* tcs = 0; // don't have texture coordinates for now
	if (tcs) delete tcs;
	if (yn == 'y') tcs = new float[vertsN * 2];

	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts
	if (cols) ifs.read((char*)cols, vertsN * 3 * sizeof(float)); // load cols
	if (normals) ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals
	if (tcs) ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	int trisN;
	ifs.read((char*)&trisN, sizeof(int));

	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();


	for (int i = 0; i < vertsN; i++) {
		cols[i] *= 255.0f;
	}

	MESH m;
	for (int i = 0; i < trisN; i++) {
		int a = tris[3 * i];
		int b = tris[3 * i + 1];
		int c = tris[3 * i + 2];

		TRIANGLE tri = TRIANGLE();
		tri.points[0] = verts[a];
		tri.points[1] = verts[b];
		tri.points[2] = verts[c];
		tri.colors[0] = COLOR(0, 200, 200);
		tri.colors[1] = COLOR(0, 200, 200);
		tri.colors[2] = COLOR(0, 200, 200);

		tri.norms = { normals[a], normals[b], normals[c] };
		tri.norm = normals[a] + normals[b] + normals[c];
		tri.norm.normalize();

		m.add_triangle(tri);
	}
	m.fill = true;
	m.translate(m.get_center() * -1.0f);
	*this = m;

	free(verts); verts = nullptr;
	free(cols); cols = nullptr;
	free(normals); normals = nullptr;
	free(tcs); tcs = nullptr;
	free(tris); tris = nullptr;
}

void MESH::SaveAsBin() {
	cout << "Saving Mesh Bin...\n";
	ofstream out(OUTPUT_BIN);
	out.write((char*)this, sizeof(MESH));
	out.close();
}

LIGHT::LIGHT() {}

LIGHT::LIGHT(V3 src, V3 direct, COLOR sh, float _a) {
	source = src;
	shade = sh;
	a = min(_a * 0.5f, (float)PI * 0.5f - 0.1f);
	cot_a = 1.0f / tan(a);

	direction = direct;
	direction.normalize();
	dir_rot = M33(1);
	V3 axis = direction;
	if (axis[Dim::Y] != 0.0f) {
		float xy_len_inverse = 1.0f / sqrt(axis[0] * axis[0] + axis[1] * axis[1]);
		float xy_cos_theta = axis[0] * xy_len_inverse;
		float xy_sin_theta = axis[1] * xy_len_inverse;
		M33 xy_rotation = M33(Dim::Z, -xy_sin_theta, xy_cos_theta);
		M33 xy_rotation_inv = M33(Dim::Z, xy_sin_theta, xy_cos_theta);
		// perform xy rotation
		axis *= xy_rotation;
		dir_rot *= xy_rotation;
	}
	if (axis[Dim::X] != 0.0f) {
		// xz rotation to eliminate axis z dimension
		float xz_len_inverse = 1.0f / sqrt(axis[0] * axis[0] + axis[2] * axis[2]);
		float xz_cos_theta = axis[0] * xz_len_inverse;
		float xz_sin_theta = axis[2] * xz_len_inverse;

		M33 xz_rotation = M33(Dim::Y, xz_sin_theta, xz_cos_theta);
		M33 xz_rotation_inv = M33(Dim::Y, -xz_sin_theta, xz_cos_theta);
		dir_rot *= xz_rotation;
	}
	dir_rot *= cot_a;
	dir_rot[Dim::Y] *= LBS_Scalar;
	dir_rot[Dim::Z] *= LBS_Scalar;
	dir_rot_inv = dir_rot.inverse();

	float cos_a = cos(a); // = direct.mag / edge
	thresold = cos_a * cos_a; // (direction * edge)^2 / (|direction||edge|)^2
}

bool LIGHT::proj(V3& point, V3& res) {
	V3 delta = point - source;
	float dot = delta * direction;
	// acute angle check
	if (dot < 0.0f) return false;
	// fov check
	if (dot * dot < thresold * (delta * delta)) return false;
	// project to shadow buffer (yz unit circle)
	V3 proj = dir_rot * delta;
	const float x_inv = 1.0f / proj[Dim::X];
	// set result
	res = V3(
		round(proj[Dim::Y] * x_inv + LBS_Scalar),
		round(proj[Dim::Z] * x_inv + LBS_Scalar),
		proj[Dim::X] // distance from light source just after rotation
	);
	return true;
}

V3 LIGHT::unproj(V3& res) {
	V3 temp = V3(
		res[Dim::Z],
		res[Dim::X] * res[Dim::Z] - LBS_Scalar,
		res[Dim::Y] * res[Dim::Z] - LBS_Scalar
	);
	V3 delta = dir_rot_inv * temp;
	return delta + source;
}

bool LIGHT::is_subject(V3& point, vector<vector<float>>& l_buffer) {
	V3 res;
	if (!proj(point, res)) return false;
	int x = (int)res[Dim::X];
	int y = (int)res[Dim::Y];
	float dist = res[Dim::Z];
	// return logic
	return dist <= l_buffer[x][y] * SM_TOLERANCE;
}

void LIGHT::check_subject_proj(V3& proj, vector<vector<float>>& l_buffer) {
	int x = (int)proj[Dim::X];
	int y = (int)proj[Dim::Y];
	float dist = proj[Dim::Z];
	l_buffer[x][y] = min(l_buffer[x][y], dist);
}

void LIGHT::check_subject(V3& point, vector<vector<float>>& l_buffer) {
	// angle check
	V3 res;
	if (!proj(point, res)) return;
	int x = (int) res[Dim::X];
	int y = (int) res[Dim::Y];
	float dist = res[Dim::Z];
	l_buffer[x][y] = min(l_buffer[x][y], dist);
}

float LIGHT::offset_lighting(V3& point, V3& norm, float phong_exp, vector<vector<float>>& l_buffer) {
	norm.normalize();
	if (!is_subject(point, l_buffer)) return 0.0f; // not hit by light -> ambient lighting
	// rotate point_to_light PI radian about normal using projection
	V3 point_to_light = source - point;
	point_to_light.normalize();
	float dot = norm * point_to_light;
	V3 reflected_light = norm * (dot * 2.0f) - point_to_light;
	reflected_light.normalize();
	// use phong's method
	V3 eye_vec = scene->ppc->C - point;
	eye_vec.normalize();
	float k_diffuse = max(dot, 0.0f);
	//cout << phong_exp << '\n';
	float k_specular = pow(eye_vec * reflected_light, phong_exp);
	return k_specular + (1.0f - scene->ambient) * k_diffuse;
}

GEOMETRY::GEOMETRY() {}

GEOMETRY::GEOMETRY(vector<GEOMETRY>& geos) {
	for (GEOMETRY& geo : geos) {
		for (int i = 0; i < geo.spheres.size(); i++)
			spheres.push_back(geo.spheres[i]);
		for (int i = 0; i < geo.segments.size(); i++)
			segments.push_back(geo.segments[i]);
		for (int i = 0; i < geo.triangles.size(); i++)
			triangles.push_back(geo.triangles[i]);
		for (int i = 0; i < geo.lights.size(); i++)
			lights.push_back(geo.lights[i]);
	}
}

void GEOMETRY::add_axis() {
	segments.push_back(
		SEGMENT(
			SPHERE(V3(-200, 0, 0)),
			SPHERE(V3(200, 0, 0))
		)
	);
	segments.push_back(
		SEGMENT(
			SPHERE(V3(0, -200, 0)),
			SPHERE(V3(0, 200, 0))
		)
	);
	segments.push_back(
		SEGMENT(
			SPHERE(V3(0, 0, -200)),
			SPHERE(V3(0, 0, 200))
		)
	);
}

void GEOMETRY::add_camera(PPC ppc) {
	V3 vd = ppc.GetVD();
	V3 c1 = ppc.C + ppc.c;
	
	V3 c2 = ppc.c;
	c2.rotate(vd, PI);
	c2 += ppc.C;

	V3 c3 = ppc.c;
	c3.rotate(vd, -1.287f);
	c3 += ppc.C;

	V3 c4 = ppc.c;
	c4.rotate(vd, -1.287f-PI);
	c4 += ppc.C;

	V3 a1 = c1 + ppc.a * 640.0f;
	V3 a2 = c2 - ppc.a * 640.0f;
	V3 b1 = a1 + ppc.b * 480.0f;
	V3 b2 = a2 - ppc.b * 480.0f;
	spheres.push_back(SPHERE(ppc.C, COLOR(255, 0, 0), 30.0f));
	segments.push_back(SEGMENT(SPHERE(ppc.C), SPHERE(c1)));
	segments.push_back(SEGMENT(SPHERE(ppc.C), SPHERE(c2)));
	segments.push_back(SEGMENT(SPHERE(ppc.C), SPHERE(c3)));
	segments.push_back(SEGMENT(SPHERE(ppc.C), SPHERE(c4)));
	segments.push_back(SEGMENT(SPHERE(c1), SPHERE(a1)));
	segments.push_back(SEGMENT(SPHERE(c2), SPHERE(a2)));
	segments.push_back(SEGMENT(SPHERE(a1), SPHERE(b1)));
	segments.push_back(SEGMENT(SPHERE(a2), SPHERE(b2)));
}

COMPUTED_GEOMETRY::COMPUTED_GEOMETRY() {
	recompute_geometry();
}

// rotate + copy geometry
void COMPUTED_GEOMETRY::recompute_geometry() {
	segments_mesh_og.clear();
	segments.clear();
	spheres.clear();
	triangles.clear();
	lights.clear();
	GEOMETRY& geometry = scene->geometry;

	for (int i = 0; i < geometry.segments.size(); i++)
		add_segment(geometry.segments[i]);
	for (int i = 0; i < geometry.triangles.size(); i++)
		add_triangle(geometry.triangles[i]);
	for (int i = 0; i < geometry.spheres.size(); i++)
		add_sphere(geometry.spheres[i]);
	for (int i = 0; i < geometry.meshes.size(); i++)
		add_mesh(geometry.meshes[i]);
	for (int i = 0; i < geometry.lights.size(); i++)
		add_light(geometry.lights[i]);
}

// rotate + translate each V3 depending on perspective + origin. 
bool COMPUTED_GEOMETRY::transform(V3& v3, V3& new_v3) {
	if (!scene->ppc->project(v3, new_v3)) {
		return false;
	}
	return true;
}

void COMPUTED_GEOMETRY::add_sphere(SPHERE& sph) {
	SPHERE new_sph = sph;
	if (transform(sph.point, new_sph.point)) {
		spheres.push_back(new_sph);
	}
}

void COMPUTED_GEOMETRY::add_segment(SEGMENT& seg) {
	SEGMENT new_seg = seg;
	if (transform(seg.start.point, new_seg.start.point) &&
		transform(seg.end.point, new_seg.end.point)) {
		segments.push_back(new_seg);
		segments_og.push_back(&seg);
	}
}

void COMPUTED_GEOMETRY::add_triangle(TRIANGLE& tri) {
	TRIANGLE new_tri = tri;
	V3& p1 = new_tri.points[0];
	V3& p2 = new_tri.points[1];
	V3& p3 = new_tri.points[2];
	if (transform(tri.points[0], p1) &&
		transform(tri.points[1], p2) &&
		transform(tri.points[2], p3)) {
		triangles.push_back(new_tri);
		triangles_og.push_back(&tri);
	}
}

void COMPUTED_GEOMETRY::add_mesh(MESH& mesh) {
	if (mesh.fill) {
		for (int i = 0; i < mesh.triangles.size(); i++) {
			add_triangle(mesh.triangles[i]);
		}
		return;
	}
	
	for (int i = 0; i < mesh.triangles.size(); i++) {
		TRIANGLE& tri = mesh.triangles[i];
		V3& op1 = tri.points[0];
		V3& op2 = tri.points[1];
		V3& op3 = tri.points[2];		
		V3 p1 = op1, p2 = op2, p3 = op3;

		if (transform(op1, p1) &&
			transform(op2, p2) &&
			transform(op3, p3)) {
			
			if (mesh.edge_connectivity[i][0] == -1) {
				segments_mesh_og.push_back(SEGMENT(op1, op2));
				segments_og.push_back(&(segments_mesh_og.back()));
				segments.push_back(SEGMENT(p1, p2));
			}
			if (mesh.edge_connectivity[i][1] == -1) {
				segments_mesh_og.push_back(SEGMENT(op2, op3));
				segments_og.push_back(&(segments_mesh_og.back()));
				segments.push_back(SEGMENT(p2, p3));
			}
			if (mesh.edge_connectivity[i][2] == -1) {
				segments_mesh_og.push_back(SEGMENT(op3, op1));
				segments_og.push_back(&(segments_mesh_og.back()));
				segments.push_back(SEGMENT(p3, p1));
			}
		}
	}
}

void COMPUTED_GEOMETRY::add_light(LIGHT& li) {
	SEGMENT* seg = new SEGMENT(
		SPHERE(li.source, COLOR(255, 255, 255)),
		SPHERE(li.source + li.direction * 50.0f, COLOR(255, 255, 255)),
		5U
	);
	SPHERE* sph = new SPHERE(li.source, COLOR(255, 255, 255), 15);
	add_sphere(*sph);
	add_segment(*seg);
	lights.push_back(li);
}