#pragma once

#include <vector>

#include "V3.hpp"

#define STROKE_WIDTH 4

class COLOR {
public:
	V3 color;

	inline unsigned int getColor();
	inline unsigned int getColor(float c);
	inline void setColor(float r, float g, float b);
	inline void setColor(V3& c);
};

class LINE3 : public COLOR {
public:
	V3 start;
	V3 end;

	LINE3();
	LINE3(V3& start, V3& end);
	LINE3(V3& start, V3& end, V3& color);
};

class POINT3 : public COLOR {
public:
	V3 point;

	POINT3();
	POINT3(V3& point);
	POINT3(V3& point, V3& color);
};

class POLY3 : public COLOR {
public:
	vector<V3> points;

	POLY3();
	POLY3(vector<V3>& points);
	POLY3(vector<V3>& points, V3& color);
};

class GEOMETRY {
public:
	vector<POINT3> points;
	vector<LINE3> lines;
	vector<POLY3> polys;

	GEOMETRY();
	GEOMETRY(GEOMETRY& geometry);
	GEOMETRY(vector<POINT3>& points, vector<LINE3>& lines, vector<POLY3>& polys);
};

class PRECOMPUTE_GEOMETRY {
public:
	vector<LINE3> lines; // transformed lines + polygon lines
	vector<POINT3> points; // transformed points
	vector<vector<int>> render_boxes; // boxes that contain geometry
	vector<bool> rendered_pixels; // union of render_boxes
	vector<V3> line_vecs; // lines: (end - start)
	vector<float> inv_dots; // lines: 1 / |end - start|^2

	PRECOMPUTE_GEOMETRY();

	inline void init_coord_rotation();
	inline void init_render_zones();
	inline void init_line_precompute();
	inline V3& transform(V3& v3);
};