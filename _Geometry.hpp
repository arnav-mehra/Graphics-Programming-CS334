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

void GEOMETRY::setup_pong() {
	num_spheres = 0;
	num_segments = 0;
	num_triangles = 0;

	{ // playing feild
		V3 corners[] = {
			V3(200, 200, 0),
			V3(200, -200, 0),
			V3(-200, -200, 0),
			V3(-200, 200, 0)
		};
		for (V3& corner : corners) {
			add_sphere(SPHERE(corner, COLOR(0, 255, 0)));
		}
		SEGMENT segs[] = {
			SEGMENT(corners[0], corners[1]),
			SEGMENT(corners[1], corners[2]),
			SEGMENT(corners[2], corners[3]),
			SEGMENT(corners[3], corners[0])
		};
		for (SEGMENT& seg : segs) {
			add_segment(seg);
		}
	}
	{ // player 1
		V3 corners[] = {
			V3(50, -200, 0) + scene->player1,
			V3(50, -190, 0) + scene->player1,
			V3(0, -190, 0) + scene->player1,
			V3(0, -200, 0) + scene->player1
		};
		SEGMENT segs[] = {
			SEGMENT(corners[0], corners[1], COLOR(255, 255, 255)),
			SEGMENT(corners[1], corners[2], COLOR(255, 255, 255)),
			SEGMENT(corners[2], corners[3], COLOR(255, 255, 255)),
			SEGMENT(corners[3], corners[0], COLOR(255, 255, 255))
		};
		for (SEGMENT& seg : segs) {
			add_segment(seg);
		}
	}
	{ // player 2
		V3 corners[] = {
			V3(50, 200, 0) + scene->player2,
			V3(50, 190, 0) + scene->player2,
			V3(0, 190, 0) + scene->player2,
			V3(0, 200, 0) + scene->player2
		};
		SEGMENT segs[] = {
			SEGMENT(corners[0], corners[1], COLOR(255, 255, 255)),
			SEGMENT(corners[1], corners[2], COLOR(255, 255, 255)),
			SEGMENT(corners[2], corners[3], COLOR(255, 255, 255)),
			SEGMENT(corners[3], corners[0], COLOR(255, 255, 255))
		};
		for (SEGMENT& seg : segs) {
			add_segment(seg);
		}
	}
	{ // ball
		V3 ball_pos = V3(0, 0, 0) + scene->ball_pos;
		add_sphere(SPHERE(ball_pos, COLOR(0, 0, 255), 10));
	}
}

void GEOMETRY::setup_tetris() {
	num_spheres = 0;
	num_segments = 0;
	num_triangles = 0;

	bool grid[20][10];
	for (int r = 0; r < 20; r++) {
		for (int c = 0; c < 10; c++) {
			grid[r][c] = scene->grid[r][c];
		}
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			int r = scene->pos.first + i;
			int c = scene->pos.second + j;
			if (r >= 0 && r < 20 && c >= 0 && c < 10)
				grid[r][c] = scene->shapes[scene->curr_shape][i][j];
		}
	}

	// border
	V3 c1 = V3(0.0f, 0.0f, 0.0f);
	V3 c2 = V3(0.0f, 400.0f, 0.0f);
	V3 c3 = V3(200.0f, 400.0f, 0.0f);
	V3 c4 = V3(200.0f, 0.0f, 0.0f);
	add_segment(SEGMENT(c1, c2));
	add_segment(SEGMENT(c2, c3));
	add_segment(SEGMENT(c3, c4));
	add_segment(SEGMENT(c4, c1));

	// draw board
	for (int r = 0; r < 20; r++) {
		for (int c = 0; c < 10; c++) {
			if (grid[r][c]) {
				V3 p1 = V3(c * 20 + 2, r * 20 + 2, 0);
				V3 p2 = V3(c * 20 + 2, r * 20 + 18, 0);
				V3 p3 = V3(c * 20 + 18, r * 20 + 18, 0);
				V3 p4 = V3(c * 20 + 18, r * 20 + 2, 0);
				V3 t1[3] = { p1, p2, p3 };
				V3 t2[3] = { p3, p4, p1 };
				add_triangle(TRIANGLE(t1));
				add_triangle(TRIANGLE(t2));
			}
		}
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
		SEGMENT& old_line = geometry.segments[i];
		V3 new_start = transform(old_line.start);
		V3 new_end = transform(old_line.end);
		SEGMENT new_line = SEGMENT(new_start, new_end, old_line.color, old_line.width);
		add_segment(new_line);
	}

	// rotate triangles. rotate each point, then pair spheres into segments
	for (int i = 0; i < geometry.num_triangles; i++) {
		TRIANGLE& triangle = geometry.triangles[i];
		V3 p[3] = {
			transform(triangle.points[0]),
			transform(triangle.points[1]),
			transform(triangle.points[2])
		};
		TRIANGLE new_tri = TRIANGLE(p, triangle.color, triangle.width);
		add_triangle(new_tri);
		/*SEGMENT s1 = SEGMENT(p[0], p[1], triangle.color, triangle.width);
		SEGMENT s2 = SEGMENT(p[1], p[2], triangle.color, triangle.width);
		SEGMENT s3 = SEGMENT(p[2], p[0], triangle.color, triangle.width);
		add_segment(s1);
		add_segment(s2);
		add_segment(s3);*/
	}

	// rotate spheres.
	for (int i = 0; i < geometry.num_spheres; i++) {
		SPHERE& p3 = geometry.spheres[i];
		V3 new_point = transform(p3.point);
		SPHERE new_sphere = SPHERE(new_point, p3.color, p3.width);
		add_sphere(new_sphere);
	}
}

// rotate + translate each V3 depending on perspective + origin. 
inline V3& COMPUTED_GEOMETRY::transform(V3& v3) {
	V3 new_v3 = scene->perspective * v3;
	new_v3 += scene->origin;
	return new_v3;
}

inline void COMPUTED_GEOMETRY::add_segment(SEGMENT& seg) {
	segments[num_segments++] = seg;
}

inline void COMPUTED_GEOMETRY::add_sphere(SPHERE& sph) {
	spheres[num_spheres++] = sph;
}

inline void COMPUTED_GEOMETRY::add_triangle(TRIANGLE& tri) {
	triangles[num_triangles++] = tri;
}