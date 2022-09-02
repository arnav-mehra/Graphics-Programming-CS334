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
		POINT3(V3(50, 50, 50), V3(0, 255, 0)),
		POINT3(V3(50, -50, 50), V3(0, 255, 0)),
		POINT3(V3(-50, 50, 50), V3(0, 255, 0)),
		POINT3(V3(-50, -50, 50), V3(0, 255, 0)),
		POINT3(V3(50, 50, -50), V3(0, 255, 0)),
		POINT3(V3(50, -50, -50), V3(0, 255, 0)),
		POINT3(V3(-50, 50, -50), V3(0, 255, 0)),
		POINT3(V3(-50, -50, -50), V3(0, 255, 0)),
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

GEOMETRY::GEOMETRY(vector<POINT3>& points) {
	this->points = points;
}

GEOMETRY::GEOMETRY(vector<LINE3>& lines) {
	this->lines = lines;
}

GEOMETRY::GEOMETRY(vector<POLY3>& polys) {
	this->polys = polys;
}

GEOMETRY::GEOMETRY(vector<POINT3>& points, vector<LINE3>& lines) {
	this->points = points;
	this->lines = lines;
}

GEOMETRY::GEOMETRY(GEOMETRY& geometry) {
	lines = geometry.lines;
	points = geometry.points;
	polys = geometry.polys;
}

GEOMETRY::GEOMETRY(vector<POINT3>& points, vector<LINE3>& lines, vector<POLY3>& polys) {
	this->lines = lines;
	this->points = points;
	this->polys = polys;
}

void GEOMETRY::add_axis() {
	lines.push_back(LINE3(V3(-200, 0, 0), V3(200, 0, 0), V3(255, 0, 0)));
	lines.push_back(LINE3(V3(0, -200, 0), V3(0, 200, 0), V3(255, 0, 0)));
	lines.push_back(LINE3(V3(0, 0, -200), V3(0, 0, 200), V3(255, 0, 0)));
}

// rotate + copy geometry
PRECOMPUTE_GEOMETRY::PRECOMPUTE_GEOMETRY() {
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

// rotate + translate each V3 depending on perspective + origin. 
inline V3& PRECOMPUTE_GEOMETRY::transform(V3& v3) {
	V3 new_v3 = scene->perspective * v3;
	new_v3 += scene->origin;
	return new_v3;
}