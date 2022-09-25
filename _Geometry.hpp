#pragma once

#include "Geometry.hpp"

#include <vector>
#include <algorithm>
#include <math.h>

#include "Dimension.hpp"
#include "M33.hpp"
#include "scene.hpp"

COLOR::COLOR() {}

COLOR::COLOR(U32 v) {
	value = v;
}

COLOR::COLOR(U32 r, U32 g, U32 b) {
	value = ((b << 16) | (g << 8) | r);
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

inline COLOR COLOR::operator*(const float scalar) {
	U32 r = (U32) (getR() * scalar);
	U32 g = (U32) (getG() * scalar);
	U32 b = (U32) (getB() * scalar);
	return COLOR(r, g, b);
}

inline void COLOR::operator*=(const float scalar) {
	(*this) = (*this) * scalar;
}

inline COLOR COLOR::operator+(const COLOR& color) {
	return COLOR(value + color.value);
}

inline COLOR COLOR::interpolate(COLOR& color, float v) {
	return (*this) * (1.0f - v) + (color) * (v);
}

COLOR INTERPOLATE::getColor(SPHERE& s1, SPHERE& s2, V3& pos) {
	float d1 = (pos - s1.point).length();
	float d2 = (pos - s2.point).length();
	float d_ = 1.0f / (d1 + d2);
	COLOR res = s1.color * (d_ * d1) + s2.color * (d_ * d2);
	return res;
}

COLOR INTERPOLATE::getColor(SPHERE& s1, SPHERE& s2, SPHERE& s3, V3& pos) {
	float d1 = (pos - s1.point).length();
	float d2 = (pos - s2.point).length();
	float d3 = (pos - s3.point).length();
	float d_ = 1.0f / (d1 + d2 + d3);
	COLOR res = s1.color * (d_ * d1) + s2.color * (d_ * d2) + s3.color * (d_ * d3);
	return res;
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

TRIANGLE::TRIANGLE() {}

TRIANGLE::TRIANGLE(vector<SPHERE> points) {
	this->points[0] = points[0];
	this->points[1] = points[1];
	this->points[2] = points[2];
	this->width = 8;
}

TRIANGLE::TRIANGLE(vector<SPHERE> points, bool isFilled) : TRIANGLE(points) {
	this->isFilled = isFilled;
}

TRIANGLE::TRIANGLE(vector<SPHERE> points, bool isFilled, U32 width) : TRIANGLE(points, isFilled) {
	this->width = width;
}

MESH::MESH() {}

MESH::MESH(vector<TRIANGLE> tris) : MESH() {
	for (TRIANGLE& tri : tris) {
		add_triangle(tri);
	}
}

MESH::MESH(vector<TRIANGLE> tris, bool renderAsWF) : MESH(tris) {
	renderAsWireFrame = renderAsWF;
}

void MESH::add_triangle(TRIANGLE tri) {
	if (num_triangles >= MESH_TRI_CAPACITY) return;

	V3& p1 = tri.points[0].point;
	V3& p2 = tri.points[1].point;
	V3& p3 = tri.points[2].point;

	edge_connectivity[num_triangles][0] = -1;
	edge_connectivity[num_triangles][1] = -1;
	edge_connectivity[num_triangles][2] = -1;

	for (int i = 0; i < num_triangles; i++) {
		V3& t1 = triangles[i].points[0].point;
		V3& t2 = triangles[i].points[1].point;
		if ((p1 == t1 || p1 == t2) && (p2 == t1 || p2 == t2)) {
			edge_connectivity[num_triangles][0] = i;
			break;
		}
	}
	for (int i = 0; i < num_triangles; i++) {
		V3& t2 = triangles[i].points[1].point;
		V3& t3 = triangles[i].points[2].point;
		if ((p2 == t2 || p2 == t3) && (p3 == t2 || p3 == t3)) {
			edge_connectivity[num_triangles][1] = i;
			break;
		}
	}
	for (int i = 0; i < num_triangles; i++) {
		V3& t1 = triangles[i].points[0].point;
		V3& t3 = triangles[i].points[2].point;
		if ((p3 == t3 || p3 == t1) && (p1 == t3 || p1 == t1)) {
			edge_connectivity[num_triangles][2] = i;
			break;
		}
	}

	triangles[num_triangles++] = tri;
}

V3 MESH::getCenter() {
	V3 sum = V3(0.0f, 0.0f, 0.0f);
	if (num_triangles == 0) return sum;
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			sum += p.point;
		}
	}
	V3 res = sum * (1.0f / ((float) num_triangles * 3.0f));
	return res;
}

void MESH::translate(V3 tran) {
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			p.point += tran;
		}
	}
}

void MESH::position(V3 pos) {
	V3 center = getCenter();
	translate(pos - center);
}

void MESH::scale(float s) {
	V3 center = getCenter();
	cout << "center" << center;
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3 delta = p.point - center;
			V3 scaled = delta * s;
			p.point = scaled + center;
		}
	}
}

void MESH::rotate(V3 axis1, V3 axis, float alpha) {
	axis -= axis1;
	float xy_len_inverse = 1 / sqrt(axis[0] * axis[0] + axis[1] * axis[1]);
	float xy_cos_theta = axis[0] * xy_len_inverse;
	float xy_sin_theta = axis[1] * xy_len_inverse;
	M33 xy_rotation = M33(Dim::Z, -xy_sin_theta, xy_cos_theta);
	M33 xy_rotation_inv = M33(Dim::Z, xy_sin_theta, xy_cos_theta);
	if (isnan(xy_len_inverse) || isinf(xy_len_inverse)) {
		xy_rotation = M33(1);
		xy_rotation_inv = M33(1);
	}
	axis = xy_rotation * axis;

	float xz_len_inverse = 1 / sqrt(axis[0] * axis[0] + axis[2] * axis[2]);
	float xz_cos_theta = axis[0] * xz_len_inverse;
	float xz_sin_theta = axis[2] * xz_len_inverse;
	M33 xz_rotation = M33(Dim::Y, xz_sin_theta, xz_cos_theta);
	M33 xz_rotation_inv = M33(Dim::Y, -xz_sin_theta, xz_cos_theta);
	if (isnan(xy_len_inverse) || isinf(xy_len_inverse)) {
		xz_rotation = M33(1);
		xz_rotation_inv = M33(1);
	}
	M33 yz_rotation = M33(Dim::X, alpha);

	M33 total_rotation = xz_rotation * xy_rotation;
	total_rotation = yz_rotation * total_rotation;
	total_rotation = xz_rotation_inv * total_rotation;
	total_rotation = xy_rotation_inv * total_rotation;

	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3& v = p.point;
			v -= axis1;
			v = total_rotation * v;
			v += axis1;
		}
	}
}

float MESH::avgDistFromCenter() {
	V3 c = getCenter();
	float total_dist = 0.0f;
	if (num_triangles == 0) return total_dist;
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3& v = p.point;
			V3 delta = v - c;
			total_dist += delta.length();
		}
	}
	return total_dist / (float) num_triangles;
}

void MESH::sphericalInterpolation(float t) {
	float avg_dist = avgDistFromCenter();
	V3 c = getCenter();
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3& v = p.point;
			V3 delta = v - c;
			float dist = (delta.length() - avg_dist) * t;
			delta.normalize();
			v -= delta * dist;
		}
	}
}

void MESH::setAsBox(V3 center, float radius) {
	num_triangles = 0;

	auto A = SPHERE(V3(1.0f, 1.0f, 1.0f));
	auto B = SPHERE(V3(1.0f, -1.0f, 1.0f), COLOR(255, 255, 0));
	auto C = SPHERE(V3(-1.0f, -1.0f, 1.0f));
	auto D = SPHERE(V3(-1.0f, 1.0f, 1.0f), COLOR(255, 255, 0));

	auto E = SPHERE(V3(1.0f, 1.0f, -1.0f), COLOR(255, 255, 0));
	auto F = SPHERE(V3(1.0f, -1.0f, -1.0f));
	auto G = SPHERE(V3(-1.0f, -1.0f, -1.0f), COLOR(255, 255, 0));
	auto H = SPHERE(V3(-1.0f, 1.0f, -1.0f));

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
	scale(radius);
	position(center);
}

void MESH::setAsSphere(V3 center, U32 hres, float radius) {
	num_triangles = 0;
	hres = min(hres, 20U);

	V3 vec = V3(0.0f, 0.0f, 1.0f);
	U32 vres = hres;
	float alpha = 2.0f * (float) PI / (float) hres;
	M33 rot1 = M33(Dim::Y, alpha);
	M33 rot2 = M33(Dim::X, -alpha);
	M33 rot(1);

	vector<vector<SPHERE>> vecs(vres, vector<SPHERE>(hres));
	for (int theta = 0; theta < vres; theta++) {
		for (int phi = 0; phi < hres; phi++) {
			vecs[theta][phi] = SPHERE(vec);
			vec = rot1 * vec;
			vec.normalize();
		}
		vec = rot2 * vec;
		vec.normalize();
	}

	for (int a = 0; a < vres; a++) {
		for (int b = 0; b < hres / 2; b++) {
			int i = (a + 1) % hres;
			int j = (b + 1) % hres;
			SPHERE& v1 = vecs[a][b];
			SPHERE& v2 = vecs[i][b];
			SPHERE& v3 = vecs[a][j];
			SPHERE& v4 = vecs[i][j];
			add_triangle(TRIANGLE({ v1, v2, v4 }));
			add_triangle(TRIANGLE({ v1, v3, v4 }));
		}
	}
	position(center);
	scale(radius);
}

void MESH::setAsCylinder(V3 center, U32 res, float height, float rad) {
	num_triangles = 0;
	res = min(res, 200U);

	float rad_inc = rad / (float) res;
	float alpha = 2.0f * (float)PI / (float)res;
	M33 rot = M33(Dim::Y, alpha);

	vector<SPHERE> upper_vecs(res);
	vector<SPHERE> lower_vecs(res);
	V3 vec = V3(rad, 0.0, 0.0);
	for (int theta = 0; theta < res; theta++) {
		V3 upper = vec;
		V3 lower = vec;
		upper[Dim::Y] += height * 0.5f;
		lower[Dim::Y] -= height * 0.5f;
		upper_vecs[theta] = SPHERE(upper);
		lower_vecs[theta] = SPHERE(lower);
		vec = rot * vec;
	}

	V3 upper_center = V3(0.0f, height * 0.5f, 0.0f);
	V3 lower_center = V3(0.0f, height * -0.5f, 0.0f);
	for (int a = 0; a < res; a++) {
		int i = (a + 1) % res;
		SPHERE& v1 = lower_vecs[a];
		SPHERE& v2 = lower_vecs[i];
		SPHERE& v3 = upper_vecs[a];
		SPHERE& v4 = upper_vecs[i];
		add_triangle(TRIANGLE({ v1, v2, v4 }));
		add_triangle(TRIANGLE({ v1, v3, v4 }));
		add_triangle(TRIANGLE({ v3, v4, upper_center }));
		add_triangle(TRIANGLE({ v1, v2, lower_center }));
	}
	position(center);
}

// load a txt file
void MESH::LoadBin() {
	cout << "Loading Mesh Bin...\n";
	ifstream in(INPUT_BIN);
	in.read((char*)this, sizeof(MESH));
	in.close();
}

// save as txt file
void MESH::SaveAsBin() {
	cout << "Saving Mesh Bin...\n";
	ofstream out(OUTPUT_BIN);
	out.write((char*)this, sizeof(MESH));
	out.close();
}

GEOMETRY::GEOMETRY() {}

GEOMETRY::GEOMETRY(vector<GEOMETRY>& geos) {
	for (GEOMETRY& geo : geos) {
		for (int i = 0; i < geo.num_spheres; i++) {
			add_sphere(geo.spheres[i]);
		}
		for (int i = 0; i < geo.num_segments; i++) {
			add_segment(geo.segments[i]);
		}
		for (int i = 0; i < geo.num_triangles; i++) {
			add_triangle(geo.triangles[i]);
		}
	}
}

GEOMETRY::GEOMETRY(vector<SPHERE> spheres, vector<SEGMENT> segments, vector<TRIANGLE> triangles, vector<MESH> meshes) {
	for (SPHERE& p : spheres) add_sphere(p);
	for (SEGMENT& s : segments) add_segment(s);
	for (TRIANGLE& t : triangles) add_triangle(t);
	for (MESH& m : meshes) add_mesh(m);
}

void GEOMETRY::add_axis() {
	add_segment(
		SEGMENT(
			SPHERE(V3(-200, 0, 0)),
			SPHERE(V3(200, 0, 0))
		)
	);
	add_segment(
		SEGMENT(
			SPHERE(V3(0, -200, 0)),
			SPHERE(V3(0, 200, 0))
		)
	);
	add_segment(
		SEGMENT(
			SPHERE(V3(0, 0, -200)),
			SPHERE(V3(0, 0, 200))
		)
	);
}

inline void GEOMETRY::add_sphere(SPHERE sph) {
	if (num_spheres < SPH_CAPACITY)
		spheres[num_spheres++] = sph;
}

inline void GEOMETRY::add_segment(SEGMENT seg) {
	if (num_segments < SEG_CAPACITY)
	segments[num_segments++] = seg;
}

inline void GEOMETRY::add_triangle(TRIANGLE tri) {
	if (num_triangles < TRI_CAPACITY)
		triangles[num_triangles++] = tri;
}

inline void GEOMETRY::add_mesh(MESH mesh) {
	if (num_meshes < MESH_CAPACITY)
		meshes[num_meshes++] = mesh;
}

COMPUTED_GEOMETRY::COMPUTED_GEOMETRY() {
	recompute_geometry();
}

// rotate + copy geometry
void COMPUTED_GEOMETRY::recompute_geometry() {
	num_segments = 0;
	num_spheres = 0;
	num_triangles = 0;
	GEOMETRY& geometry = scene->geometry;

	for (int i = 0; i < geometry.num_segments; i++) {
		add_segment(geometry.segments[i]);
	}
	for (int i = 0; i < geometry.num_triangles; i++) {
		add_triangle(geometry.triangles[i]);
	}
	for (int i = 0; i < geometry.num_spheres; i++) {
		add_sphere(geometry.spheres[i]);
	}
	for (int i = 0; i < geometry.num_meshes; i++) {
		add_mesh(geometry.meshes[i]);
	}
}

// rotate + translate each V3 depending on perspective + origin. 
inline bool COMPUTED_GEOMETRY::transform(V3& v3, V3& new_v3) {
	if (!scene->ppc->Project(v3, new_v3)) {
		return false;
	}
	return true;
}

inline void COMPUTED_GEOMETRY::add_sphere(SPHERE& sph) {
	SPHERE& new_sph = spheres[num_spheres];
	if (transform(sph.point, new_sph.point)) {
		new_sph.color = sph.color;
		new_sph.width = sph.width;
		num_spheres++; // save new sphere
	}
}

inline void COMPUTED_GEOMETRY::add_segment(SEGMENT& seg) {
	SEGMENT& new_seg = segments[num_segments];
	if (transform(seg.start.point, new_seg.start.point) &&
		transform(seg.end.point, new_seg.end.point)) {

		new_seg.start.color = seg.start.color;
		new_seg.start.width = seg.start.width;
		new_seg.end.color = seg.end.color;
		new_seg.end.width = seg.end.width;
		new_seg.width = seg.width;
		num_segments++; // save new segment
	}
}

inline void COMPUTED_GEOMETRY::add_triangle(TRIANGLE& tri) {
	TRIANGLE& new_tri = triangles[num_triangles];
	if (transform(tri.points[0].point, new_tri.points[0].point) &&
		transform(tri.points[1].point, new_tri.points[1].point) &&
		transform(tri.points[2].point, new_tri.points[2].point)) {

		new_tri.points[0].color = tri.points[0].color;
		new_tri.points[0].width = tri.points[0].width;
		new_tri.points[1].color = tri.points[1].color;
		new_tri.points[1].width = tri.points[1].width;
		new_tri.points[2].color = tri.points[2].color;
		new_tri.points[2].width = tri.points[2].width;
		num_triangles++; // save new triangle
	}
}

inline void COMPUTED_GEOMETRY::add_mesh(MESH& mesh) {
	if (!mesh.renderAsWireFrame) {
		for (int i = 0; i < mesh.num_triangles; i++) {
			add_triangle(mesh.triangles[i]);
		}
		return;
	}
	
	for (int i = 0; i < mesh.num_triangles; i++) {
		TRIANGLE& tri = mesh.triangles[i];
		SPHERE p1, p2, p3;
			
		if (transform(tri.points[0].point, p1.point) &&
			transform(tri.points[1].point, p2.point) &&
			transform(tri.points[2].point, p3.point)) {

			p1.color = tri.points[0].color;
			p2.color = tri.points[1].color;
			p3.color = tri.points[2].color;
				
			if (mesh.edge_connectivity[i][0] == -1)
				segments[num_segments++] = SEGMENT(p1, p2);
			if (mesh.edge_connectivity[i][1] == -1)
				segments[num_segments++] = SEGMENT(p2, p3);
			if (mesh.edge_connectivity[i][2] == -1)
				segments[num_segments++] = SEGMENT(p3, p1);
		}
	}
}