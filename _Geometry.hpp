#pragma once

#include "Geometry.hpp"

#include <vector>
#include <algorithm>

#include "Dimension.hpp"
#include "M33.hpp"
#include "scene.h"

inline U32 GEO_META::scaleColor(float scalar) {
	U32 r = (color & 255) * scalar;
	U32 g = ((color >> 8) & 255) * scalar;
	U32 b = ((color >> 16) & 255) * scalar;
	return COLOR(r, g, b);
}

SPHERE::SPHERE() {}

SPHERE::SPHERE(V3& point) {
	this->point = point;
	this->width = 4;
	this->color = COLOR(255, 0, 0);
}

SPHERE::SPHERE(V3& point, U32 color) {
	this->point = point;
	this->color = color;
	this->width = 4;
}

SPHERE::SPHERE(V3& point, U32 color, U32 width) {
	this->point = point;
	this->color = color;
	this->width = width;
}

SEGMENT::SEGMENT() {}

SEGMENT::SEGMENT(V3& start, V3& end) {
	this->start = start;
	this->end = end;
	this->width = 4;
	this->color = COLOR(255, 0, 0);
}

SEGMENT::SEGMENT(V3& start, V3& end, U32 color) {
	this->start = start;
	this->end = end;
	this->width = 4;
	this->color = color;
}

SEGMENT::SEGMENT(V3& start, V3& end, U32 color, U32 width) {
	this->start = start;
	this->end = end;
	this->width = width;
	this->color = color;
}

TRIANGLE::TRIANGLE() {}

TRIANGLE::TRIANGLE(V3(&spheres)[3]) {
	this->spheres[0] = spheres[0];
	this->spheres[1] = spheres[1];
	this->spheres[2] = spheres[2];
	this->color = COLOR(255, 0, 0);
	this->width = 4;
}

TRIANGLE::TRIANGLE(V3(&spheres)[3], U32 color) {
	this->spheres[0] = spheres[0];
	this->spheres[1] = spheres[1];
	this->spheres[2] = spheres[2];
	this->color = color;
	this->width = 4;
}

TRIANGLE::TRIANGLE(V3(&spheres)[3], U32 color, U32 width) {
	this->spheres[0] = spheres[0];
	this->spheres[1] = spheres[1];
	this->spheres[2] = spheres[2];
	this->color = color;
	this->width = width;
}

GEOMETRY::GEOMETRY() {
	V3 v[] = {
		V3(50, 50, 50),
		V3(50, -50, 50),
		V3(-50, 50, 50),
		V3(-50, -50, 50),
		V3(50, 50, -50),
		V3(50, -50, -50),
		V3(-50, 50, -50),
		V3(-50, -50, -50)
	};
	for (int i = 0; i < 8; i++) {
		add_sphere(SPHERE(v[i], COLOR(0, 255, 0)));
	}
	SEGMENT segs[] = {
		SEGMENT(v[0], v[1]),
		SEGMENT(v[1], v[2]),
		SEGMENT(v[2], v[3]),
		SEGMENT(v[3], v[0]),
		SEGMENT(v[4], v[5]),
		SEGMENT(v[5], v[6]),
		SEGMENT(v[6], v[7]),
		SEGMENT(v[7], v[4]),
		SEGMENT(v[0], v[4]),
		SEGMENT(v[1], v[5]),
		SEGMENT(v[2], v[6]),
		SEGMENT(v[3], v[7]),
	};
	for (SEGMENT& seg : segs) {
		add_segment(seg);
	}
}

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

GEOMETRY::GEOMETRY(vector<SPHERE>& spheres, vector<SEGMENT>& segments, vector<TRIANGLE>& triangles) {
	for (SPHERE& p : spheres) add_sphere(p);
	for (SEGMENT& s : segments) add_segment(s);
	for (TRIANGLE& t : triangles) add_triangle(t);
}

GEOMETRY::GEOMETRY(vector<SPHERE> spheres, vector<SEGMENT> segments, vector<TRIANGLE> triangles) {
	for (SPHERE& p : spheres) add_sphere(p);
	for (SEGMENT& s : segments) add_segment(s);
	for (TRIANGLE& t : triangles) add_triangle(t);
}

void GEOMETRY::add_axis() {
	add_segment(SEGMENT(V3(-200, 0, 0), V3(200, 0, 0)));
	add_segment(SEGMENT(V3(0, -200, 0), V3(0, 200, 0)));
	add_segment(SEGMENT(V3(0, 0, -200), V3(0, 0, 200)));
}

inline void GEOMETRY::add_segment(SEGMENT& seg) {
	segments[num_segments++] = seg;
}

inline void GEOMETRY::add_sphere(SPHERE& sph) {
	spheres[num_spheres++] = sph;
}

inline void GEOMETRY::add_triangle(TRIANGLE& tri) {
	triangles[num_triangles++] = tri;
}

PRECOMPUTE_GEOMETRY::PRECOMPUTE_GEOMETRY() {
	recompute_geometry();
}

// rotate + copy geometry
void PRECOMPUTE_GEOMETRY::recompute_geometry() {
	num_segments = 0;
	num_spheres = 0;
	GEOMETRY& geometry = scene->geometry;

	// rotate segments to showcase 3D.
	for (int i = 0; i < geometry.num_segments; i++) {
		SEGMENT& old_line = geometry.segments[i];
		V3 new_start = transform(old_line.start);
		V3 new_end = transform(old_line.end);
		SEGMENT new_line = SEGMENT(new_start, new_end, old_line.color, old_line.width);
		add_segment(new_line);
	}

	// rotate triangles. rotate each point, then pair spheres into segments
	for (int i = 0; i < geometry.num_triangles; i++) {
		TRIANGLE& triangle = geometry.triangles[i];
		V3 p1 = transform(triangle.spheres[0]);
		V3 p2 = transform(triangle.spheres[1]);
		V3 p3 = transform(triangle.spheres[2]);
		add_segment(SEGMENT(p1, p2, triangle.color, triangle.width));
		add_segment(SEGMENT(p2, p3, triangle.color, triangle.width));
		add_segment(SEGMENT(p3, p1, triangle.color, triangle.width));
	}

	// rotate spheres.
	for (int i = 0; i < geometry.num_spheres; i++) {
		SPHERE& p3 = geometry.spheres[i];
		add_sphere(SPHERE(transform(p3.point), p3.color, p3.width));
	}
}

// rotate + translate each V3 depending on perspective + origin. 
inline V3& PRECOMPUTE_GEOMETRY::transform(V3& v3) {
	V3 new_v3 = scene->perspective * v3;
	new_v3 += scene->origin;
	return new_v3;
}

inline void PRECOMPUTE_GEOMETRY::add_segment(SEGMENT& seg) {
	segments[num_segments++] = seg;
}

inline void PRECOMPUTE_GEOMETRY::add_sphere(SPHERE& sph) {
	spheres[num_spheres++] = sph;
}