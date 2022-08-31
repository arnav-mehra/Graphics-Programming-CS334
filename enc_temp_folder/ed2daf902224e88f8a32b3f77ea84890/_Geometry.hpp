#pragma once

#include "Geometry.hpp"

#include <vector>
#include <algorithm>

#include "Dimension.hpp"
#include "M33.hpp"
#include "scene.h"

#define STROKE_WIDTH 4

inline unsigned int COLOR::getColor() {
	unsigned int r = (unsigned int) color[0];
	unsigned int g = (unsigned int) color[1];
	unsigned int b = (unsigned int) color[2];
	return (b << 16) | (g << 8) | r;
}

inline unsigned int COLOR::getColor(float c) {
	unsigned int r = (unsigned int) (color[0] * c);
	unsigned int g = (unsigned int) (color[1] * c);
	unsigned int b = (unsigned int) (color[2] * c);
	return (b << 16) | (g << 8) | r;
}

inline void COLOR::setColor(V3& c) {
	color = c;
}

inline void COLOR::setColor(float r, float g, float b) {
	color = V3(r, g, b);
}

LINE3::LINE3() {}

LINE3::LINE3(V3& start, V3& end) {
	this->start = start;
	this->end = end;
}

LINE3::LINE3(V3& start, V3& end, V3& color) {
	this->start = start;
	this->end = end;
	this->color = color;
}

POINT3::POINT3() {}

POINT3::POINT3(V3& point) {
	this->point = point;
}

POINT3::POINT3(V3& point, V3& color) {
	this->point = point;
	this->color = color;
}

POLY3::POLY3() {}

POLY3::POLY3(vector<V3>& points) {
	this->points = points;
}

POLY3::POLY3(vector<V3>& points, V3& color) {
	this->points = points;
	this->color = color;
}

GEOMETRY::GEOMETRY() {
	points = {
	};
	lines = {
		LINE3(V3(-200, 0, 0), V3(200, 0, 0), V3(255, 0, 0)),
		LINE3(V3(0, -200, 0), V3(0, 200, 0), V3(255, 0, 0)),
		LINE3(V3(0, 0, -200), V3(0, 0, 200), V3(255, 0, 0))
	};
	vector<V3> pv = {
		V3(0, 0, 0),
		V3(100, 0, 0),
		V3(100, 0, 100),
		V3(100, 0, 0),

		V3(100, 100, 0),
		V3(100, 100, 100),
		V3(100, 100, 0),

		V3(0, 100, 0),
		V3(0, 100, 100),
		V3(0, 100, 0),

		V3(0, 0, 0),
		V3(0, 0, 100),
		V3(100, 0, 100),
		V3(100, 100, 100),
		V3(0, 100, 100),
		V3(0, 0, 100),
	};
	for (V3& v : pv) v -= V3(50, 50, 50);
	polys = {
		POLY3(pv, V3(0, 255, 0))
	};
}

GEOMETRY::GEOMETRY(GEOMETRY& geometry) {
	lines = geometry.lines;
	points = geometry.points;
	polys = geometry.polys;
}

GEOMETRY::GEOMETRY(vector<POINT3>& points, vector<LINE3>& lines, vector<POLY3>& polys) {
	lines = lines;
	points = points;
	polys = polys;
}

PRECOMPUTE_GEOMETRY::PRECOMPUTE_GEOMETRY() {
	init_coord_rotation();
	init_render_zones();
	init_line_precompute();
}

// rotate + copy geometry
inline void PRECOMPUTE_GEOMETRY::init_coord_rotation() {
	GEOMETRY& geometry = scene->geometry;

	// resize vectors
	int num_lines = (int) geometry.lines.size();
	for (POLY3& poly : geometry.polys) {
		int lines = (int) poly.points.size() - 1;
		num_lines += lines > 0 ? lines : 0;
	}
	lines.resize(num_lines);
	points.resize(geometry.points.size());

	// rotate lines to showcase 3D.
	int line_i = 0;
	for (LINE3& old_line : geometry.lines) {
		LINE3& new_line = lines[line_i++];
		new_line.color = old_line.color;
		new_line.start = transform(old_line.start);
		new_line.end = transform(old_line.end);
	}

	// rotate polys. rotate each point, then pair points into lines
	for (POLY3& poly : geometry.polys) {
		vector<V3> points(poly.points.size());
		for (int i = 0; i < points.size(); i++) {
			V3 temp = transform(poly.points[i]);
			points[i] = temp;
		}
		for (int i = 0; i < points.size() - 1; i++) {
			LINE3& new_line = lines[line_i++];
			new_line = LINE3(points[i], points[i + 1], poly.color);
		}
	}

	// rotate points.
	for (int i = 0; i < geometry.points.size(); i++) {
		POINT3& p3 = geometry.points[i];
		V3 new_point = transform(p3.point);
		points[i] = POINT3(new_point, p3.color);
	}
}

// create and determine pixels to render
inline void PRECOMPUTE_GEOMETRY::init_render_zones() {
	for (LINE3& line : lines) {
		V3& start = line.start;
		V3& end = line.end;

		int min_x = (int) min(start[Dim::X], end[Dim::X]) - (STROKE_WIDTH / 2);
		if (min_x < 0) min_x = 0;
		int min_y = (int) min(start[Dim::Y], end[Dim::Y]) - (STROKE_WIDTH / 2);
		if (min_y < 0) min_y = 0;
		int max_x = (int) max(start[Dim::X], end[Dim::X]) + (STROKE_WIDTH / 2);
		if (max_x >= scene->w) max_x = scene->w - 1;
		int max_y = (int) max(start[Dim::Y], end[Dim::Y]) + (STROKE_WIDTH / 2);
		if (max_y >= scene->h) max_y = scene->h - 1;

		for (int y = min_y; y <= max_y; y++) {
			int row = y * scene->w;
			for (int x = min_x; x <= max_x; x++) {
				int p = row + x;
				rendered_pixels[p] = true;
			}
		}
	}
}

// line_vec and inv dot precompute
inline void PRECOMPUTE_GEOMETRY::init_line_precompute() {
	line_vecs.resize(lines.size());
	inv_dots.resize(lines.size());

	for (int i = 0; i < lines.size(); i++) {
		V3& start = lines[i].start;
		V3& end = lines[i].end;
		V3 line_vec = end - start;
		line_vecs[i] = line_vec;
		inv_dots[i] = 1 / (line_vec * line_vec);
	}
}

// rotate + translate each V3 depending on perspective + origin. 
inline V3& PRECOMPUTE_GEOMETRY::transform(V3& v3) {
	V3 new_v3 = scene->perspective * v3;
	new_v3 += scene->origin;
	new_v3[2] = 0;
	return new_v3;
}