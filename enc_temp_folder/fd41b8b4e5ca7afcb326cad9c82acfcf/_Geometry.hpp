#pragma once

#include "Geometry.hpp"

#include <vector>
#include <algorithm>

#include "Dimension.hpp"
#include "M33.hpp"
#include "scene.h"

COLOR::COLOR() {}

COLOR::COLOR(U32 v) {
	value = v;
}

COLOR::COLOR(U32 r, U32 g, U32 b) {
	value = ((b << 24) | (g << 16) | (r << 8));
}

U32 COLOR::getR() const {
	return (value >> 8) & 255;
}

U32 COLOR::getG() const {
	return (value >> 16) & 255;
}

U32 COLOR::getB() const {
	return (value >> 24) & 255;
}

inline COLOR COLOR::operator*(const float scalar) {
	U32 r = getR() * scalar;
	U32 g = getG() * scalar;
	U32 b = getB() * scalar;
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

TRIANGLE::TRIANGLE(vector<SPHERE> points, U32 width) : TRIANGLE(points) {
	this->width = width;
}

MESH::MESH() {}

MESH::MESH(vector<TRIANGLE> tris) : MESH() {
	for (TRIANGLE& tri : tris) {
		triangles[num_triangles++] = tri;
	}
}

MESH::MESH(vector<TRIANGLE> tris, bool renderAsWF) : MESH(tris) {
	renderAsWireFrame = renderAsWF;
}

V3 MESH::getCenter() {
	V3 sum = V3(0.0f, 0.0f, 0.0f);
	if (num_triangles == 0) return sum;
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			sum += p.point;
		}
	}
	V3 res = sum * (1.0f / (float) num_triangles);
	return res;
}

void MESH::translate(V3 tran) {
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			p.point += tran;
		}
	}
}

void MESH::scale(float s) {
	V3 center = getCenter();
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3 delta = p.point - center;
			V3 scaled = delta * s;
			p.point = scaled + center;
		}
	}
}

void MESH::setAsBox() {
	auto A = SPHERE(V3(100.0f, 100.0f, -200.0f));
	auto B = SPHERE(V3(100.0f, 0.0f, -200.0f), COLOR(255, 255, 0));
	auto C = SPHERE(V3(0.0f, 0.0f, -200.0f));
	auto D = SPHERE(V3(0.0f, 100.0f, -200.0f), COLOR(255, 255, 0));

	auto E = SPHERE(V3(100.0f, 100.0f, -300.0f), COLOR(255, 255, 0));
	auto F = SPHERE(V3(100.0f, 0.0f, -300.0f));
	auto G = SPHERE(V3(0.0f, 0.0f, -300.0f), COLOR(255, 255, 0));
	auto H = SPHERE(V3(0.0f, 100.0f, -300.0f));

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
	spheres[num_spheres++] = sph;
}

inline void GEOMETRY::add_segment(SEGMENT seg) {
	segments[num_segments++] = seg;
}

inline void GEOMETRY::add_triangle(TRIANGLE tri) {
	triangles[num_triangles++] = tri;
}

inline void GEOMETRY::add_mesh(MESH mesh) {
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
	if (mesh.renderAsWireFrame) {
		for (int i = 0; i < mesh.num_triangles; i++) {
			TRIANGLE& tri = mesh.triangles[i];
			SPHERE p1, p2, p3;

			cout << "try\n";
			if (transform(tri.points[0].point, p1.point) &&
				transform(tri.points[1].point, p2.point) &&
				transform(tri.points[2].point, p3.point)) {
				cout << "done\n";
				p1.color = tri.points[0].color;
				p2.color = tri.points[1].color;
				p3.color = tri.points[2].color;
				
				segments[num_segments++] = SEGMENT(p1, p2);
				segments[num_segments++] = SEGMENT(p2, p3);
				segments[num_segments++] = SEGMENT(p3, p1);
			}
		}
	}
}