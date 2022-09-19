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

TRIANGLE::TRIANGLE(V3(&points)[3]) {
	this->points[0] = points[0];
	this->points[1] = points[1];
	this->points[2] = points[2];
	this->color = COLOR(255, 0, 0);
	this->width = 4;
}

TRIANGLE::TRIANGLE(V3(&points)[3], U32 color) {
	this->points[0] = points[0];
	this->points[1] = points[1];
	this->points[2] = points[2];
	this->color = color;
	this->width = 4;
}

TRIANGLE::TRIANGLE(V3(&points)[3], U32 color, U32 width) {
	this->points[0] = points[0];
	this->points[1] = points[1];
	this->points[2] = points[2];
	this->color = color;
	this->width = width;
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

inline void GEOMETRY::add_segment(SEGMENT seg) {
	segments[num_segments++] = seg;
}

inline void GEOMETRY::add_sphere(SPHERE sph) {
	spheres[num_spheres++] = sph;
}

inline void GEOMETRY::add_triangle(TRIANGLE tri) {
	triangles[num_triangles++] = tri;
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

	// rotate segments to showcase 3D.
	for (int i = 0; i < geometry.num_segments; i++) {
		add_segment(geometry.segments[i]);
	}

	// rotate triangles. rotate each point, then pair spheres into segments
	for (int i = 0; i < geometry.num_triangles; i++) {
		add_triangle(geometry.triangles[i]);
	}

	// rotate spheres.
	for (int i = 0; i < geometry.num_spheres; i++) {
		add_sphere(geometry.spheres[i]);
	}
}

// rotate + translate each V3 depending on perspective + origin. 
inline bool COMPUTED_GEOMETRY::transform(V3& v3, V3& new_v3) {
	if (!scene->ppc->Project(v3, new_v3)) {
		return false;
	}
	return true;
}

inline void COMPUTED_GEOMETRY::add_segment(SEGMENT& seg) {
	//segments[num_segments++] = seg;
	
	V3 new_start; V3 new_end;
	if (transform(seg.start, new_start) &&
		transform(seg.end, new_end)) {
		segments[num_segments++] = SEGMENT(new_start, new_end, seg.color, seg.width);
	}
}

inline void COMPUTED_GEOMETRY::add_sphere(SPHERE& sph) {
	//spheres[num_spheres++] = sph;

	V3 new_point;
	if (transform(sph.point, new_point)) {
		spheres[num_spheres++] = SPHERE(new_point, sph.color, sph.width);
	}
}

inline void COMPUTED_GEOMETRY::add_triangle(TRIANGLE& tri) {
	//triangles[num_triangles++] = tri;

	V3 p[3];
	if (transform(tri.points[0], p[0]) &&
		transform(tri.points[1], p[1]) &&
		transform(tri.points[2], p[2])) {
		triangles[num_triangles++] = TRIANGLE(p, tri.color, tri.width);
	}
	/*SEGMENT s1 = SEGMENT(p[0], p[1], triangle.color, triangle.width);
	SEGMENT s2 = SEGMENT(p[1], p[2], triangle.color, triangle.width);
	SEGMENT s3 = SEGMENT(p[2], p[0], triangle.color, triangle.width);
	add_segment(s1);
	add_segment(s2);
	add_segment(s3);*/
}