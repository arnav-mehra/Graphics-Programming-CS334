#pragma once

#include <vector>

#include "V3.hpp"

#define COLOR(r,g,b) (((b) << 16) | ((g) << 8) | (r))

typedef unsigned int U32;

class GEO_META {
public:
	U32 width;
	U32 color;
	inline U32 scaleColor(float c);
};

class SEGMENT : public GEO_META {
public:
	V3 start;
	V3 end;

	SEGMENT();
	SEGMENT(V3& start, V3& end);
	SEGMENT(V3& start, V3& end, U32 color);
	SEGMENT(V3& start, V3& end, U32 color, U32 width);
};

class SPHERE : public GEO_META {
public:
	V3 point;

	SPHERE();
	SPHERE(V3& point);
	SPHERE(V3& point, U32 color);
	SPHERE(V3& point, U32 color, U32 width);
};

class TRIANGLE : public GEO_META {
public:
	V3 points[3];

	TRIANGLE();
	TRIANGLE(V3 (&points)[3]);
	TRIANGLE(V3 (&points)[3], U32 color);
	TRIANGLE(V3 (&points)[3], U32 color, U32 width);
};

class GEOMETRY {
public:
	int num_spheres = 0;
	int num_segments = 0;
	int num_triangles = 0;
	SPHERE spheres[1000];
	SEGMENT segments[1000];
	TRIANGLE triangles[1000];

	// preloaded geometry (check function for details)
	GEOMETRY();
	
	GEOMETRY(vector<GEOMETRY>& geos);

	GEOMETRY(vector<SPHERE> spheres, vector<SEGMENT> segments, vector<TRIANGLE> triangles);

	void setup_pong();
	void setup_tetris();
	void add_axis();

	inline void add_segment(SEGMENT seg);
	inline void add_sphere(SPHERE sph);
	inline void add_triangle(TRIANGLE tri);
};

class COMPUTED_GEOMETRY {
public:
	int num_segments = 0;
	int num_spheres = 0;
	int num_triangles = 0;
	SEGMENT segments[4000]; // transformed segs + triangle segs
	SPHERE spheres[1000]; // transformed spheres
	TRIANGLE triangles[1000];

	COMPUTED_GEOMETRY();

	void recompute_geometry();

	// rotate and translate point based on perspective and origin.
	inline V3& transform(V3& v3);

	inline void add_segment(SEGMENT& seg);
	inline void add_sphere(SPHERE& sph);
	inline void add_triangle(TRIANGLE& tri);
};