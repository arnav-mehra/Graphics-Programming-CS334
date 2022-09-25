#pragma once

#include <vector>
#include "V3.hpp"

#define INPUT_BIN "geo.bin"
#define OUTPUT_BIN "geo.bin"
#define IO_MESH 0 // changes index of mesh in geometry that gets read/written and also rotated.

// NOTE: Changing below parameters will mess up geometry file IO.
#define SEG_CAPACITY 12000
#define SPH_CAPACITY 100
#define TRI_CAPACITY 10
#define MESH_CAPACITY 4
#define MESH_TRI_CAPACITY 1000

typedef unsigned int U32;

class GEO_META {
public:
	U32 width;
};

class COLOR {
public:
	U32 value = 0;

	COLOR();
	COLOR(U32 v);
	COLOR(U32 r, U32 g, U32 b);
	
	inline COLOR operator*(const float c);
	inline void operator*=(const float c);
	inline COLOR operator+(const COLOR& color);
	inline COLOR interpolate(COLOR& color, float v);

	U32 getR() const;
	U32 getG() const;
	U32 getB() const;
};

class SPHERE : public GEO_META {
public:
	V3 point;
	COLOR color;

	SPHERE();
	SPHERE(V3 point);
	SPHERE(V3 point, COLOR color);
	SPHERE(V3 point, COLOR color, U32 width);	
};

class INTERPOLATE {
	static inline COLOR getColor(SPHERE& s1, SPHERE& s2, V3& pos);
	static inline COLOR getColor(SPHERE& s1, SPHERE& s2, SPHERE& s3, V3& pos);
};

class SEGMENT : public GEO_META {
public:
	SPHERE start;
	SPHERE end;

	SEGMENT();
	SEGMENT(SPHERE start, SPHERE end);
	SEGMENT(SPHERE start, SPHERE end, U32 width);
};

class TRIANGLE : public GEO_META {
public:
	SPHERE points[3];
	bool isFilled;

	TRIANGLE();
	TRIANGLE(vector<SPHERE> points);
	TRIANGLE(vector<SPHERE> points, bool isFilled);
	TRIANGLE(vector<SPHERE> points, bool isFilled, U32 width);
};

class MESH {
public:
	// NOTE: EACH TRIANGLE HAS ITS VECTORS + COLOR CONTAINED.
	TRIANGLE triangles[MESH_TRI_CAPACITY];
	int edge_connectivity[MESH_TRI_CAPACITY][3];
	int num_triangles = 0;
	bool renderAsWireFrame = true;

	MESH();
	MESH(vector<TRIANGLE> tris);
	MESH(vector<TRIANGLE> tris, bool renderAsWF);

	void add_triangle(TRIANGLE tri);

	V3 getCenter();
	float avgDistFromCenter();
	void sphericalInterpolation(float t);
	void translate(V3 tran);
	void position(V3 pos);
	void scale(float s);
	void rotate(V3 axis1, V3 axis2, float alpha);

	void setAsBox(V3 center, float radius);
	void setAsSphere(V3 center, U32 resolution, float radius);
	void setAsCylinder(V3 center, U32 res, float height, float radius);

	void SaveAsBin();
	void LoadBin();
};

class GEOMETRY {
public:
	int num_spheres = 0;
	int num_segments = 0;
	int num_triangles = 0;
	int num_meshes = 0;
	SPHERE spheres[SPH_CAPACITY];
	SEGMENT segments[SEG_CAPACITY];
	TRIANGLE triangles[TRI_CAPACITY];
	MESH meshes[MESH_CAPACITY];

	GEOMETRY();
	GEOMETRY(vector<GEOMETRY>& geos);
	GEOMETRY(vector<SPHERE> spheres,
			 vector<SEGMENT> segments,
			 vector<TRIANGLE> triangles,
			 vector<MESH> meshes);

	void add_axis();
	inline void add_sphere(SPHERE sph);
	inline void add_segment(SEGMENT seg);
	inline void add_triangle(TRIANGLE tri);
	inline void add_mesh(MESH mesh);
};

class COMPUTED_GEOMETRY {
public:
	int num_segments = 0;
	int num_spheres = 0;
	int num_triangles = 0;
	SPHERE spheres[SPH_CAPACITY];
	SEGMENT segments[SEG_CAPACITY];
	TRIANGLE triangles[TRI_CAPACITY];

	COMPUTED_GEOMETRY();

	void recompute_geometry();

	// transform point based on PPC.
	inline bool transform(V3& v3, V3& new_v3);

	inline void add_sphere(SPHERE& sph);
	inline void add_segment(SEGMENT& seg);
	inline void add_triangle(TRIANGLE& tri);
	inline void add_mesh(MESH& mesh);
};