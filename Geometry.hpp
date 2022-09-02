#pragma once

#include <vector>

#include "V3.hpp"

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

	// preloaded geometry (check function for details)
	GEOMETRY();

	GEOMETRY(GEOMETRY& geometry);

	GEOMETRY(vector<POINT3>& points);
	GEOMETRY(vector<LINE3>& lines);
	GEOMETRY(vector<POLY3>& polys);

	GEOMETRY(vector<POINT3>& points, vector<LINE3>& lines);
	GEOMETRY(vector<POINT3>& points, vector<LINE3>& lines, vector<POLY3>& polys);

	void add_axis();
};

class PRECOMPUTE_GEOMETRY {
public:
	vector<LINE3> lines; // transformed lines + polygon lines
	vector<POINT3> points; // transformed points

	PRECOMPUTE_GEOMETRY();

	inline V3& transform(V3& v3);
};