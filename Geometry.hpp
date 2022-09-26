#pragma once

#include <vector>

#include "V3.hpp"

#define INPUT_BIN "geo.bin"
#define OUTPUT_BIN "geo.bin"
#define SEL_MESH 0 // changes index of mesh in geometry that gets read/written and also rotated.
#define SEL_LIGHT 0 // light that is moved 

// NOTE: Changing below parameters will mess up geometry file IO.
#define SEG_CAPACITY 12000
#define SPH_CAPACITY 1000
#define TRI_CAPACITY 4000
#define MESH_CAPACITY 4
#define MESH_TRI_CAPACITY 1000
#define LIGHT_CAPACITY 1

#define K_AMBIENT 1.0f

typedef unsigned int U32;

using namespace std;

class GEO_META {
public:
	U32 width;
	float phong_exp = 200.0f;
};

class COLOR {
public:
	U32 value = 0;

	COLOR();
	COLOR(U32 v);
	COLOR(U32 r, U32 g, U32 b);
	
	COLOR operator*(const float c);
	void operator*=(const float c);
	COLOR operator+(const COLOR& color);
	COLOR interpolate(COLOR& color, float v);

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

	TRIANGLE();
	TRIANGLE(vector<SPHERE> points);
	TRIANGLE(vector<SPHERE> points, U32 width);
};

class INTERPOLATE {
public:
	static COLOR getColor(SEGMENT& seg, V3& pos);
	static COLOR getColor(TRIANGLE& tri, V3& pos);
};

class MESH {
public:
	// NOTE: EACH TRIANGLE HAS ITS VECTORS + COLOR CONTAINED.
	TRIANGLE triangles[MESH_TRI_CAPACITY];
	int edge_connectivity[MESH_TRI_CAPACITY][3];
	int num_triangles = 0;
	bool fill;

	MESH();
	MESH(vector<TRIANGLE> tris);
	MESH(vector<TRIANGLE> tris, bool fill);

	void add_triangle(TRIANGLE tri);

	V3 get_center();
	float avg_dist_from_center();
	
	void translate(V3 tran);
	void position(V3 pos);
	void scale(float s);
	void rotate(V3 axis1, V3 axis2, float alpha);

	void setAsBox(V3 center, float radius);
	void setAsSphere(V3 center, U32 resolution, float radius);
	void setAsCylinder(V3 center, U32 res, float height, float radius);

	void spherical_interpolation(float t, float res);

	void SaveAsBin();
	void LoadBin();
	void Load334Bin();
};

class LIGHT {
public:
	V3 source;
	V3 direction;
	COLOR shade;
	float thresold;

	LIGHT();
	LIGHT(V3 src, V3 direct, COLOR sh, float a);

	bool is_subject(V3& point);
	float offset_lighting(V3& point, V3& norm, float phong_exp);
};

class GEOMETRY {
public:
	int num_spheres = 0;
	int num_segments = 0;
	int num_triangles = 0;
	int num_meshes = 0;
	int num_lights = 0;
	SPHERE spheres[SPH_CAPACITY];
	SEGMENT segments[SEG_CAPACITY];
	TRIANGLE triangles[TRI_CAPACITY];
	MESH meshes[MESH_CAPACITY];
	LIGHT lights[LIGHT_CAPACITY];

	GEOMETRY();
	GEOMETRY(vector<GEOMETRY>& geos);
	GEOMETRY(vector<SPHERE> spheres,
			 vector<SEGMENT> segments,
			 vector<TRIANGLE> triangles,
			 vector<MESH> meshes,
			 vector<LIGHT> lights);

	void add_axis();
	void add_sphere(SPHERE sph);
	void add_segment(SEGMENT seg);
	void add_triangle(TRIANGLE tri);
	void add_mesh(MESH mesh);
	void add_light(LIGHT li);
};

class COMPUTED_GEOMETRY {
public:
	int num_segments = 0;
	int num_spheres = 0;
	int num_triangles = 0;
	int num_lights = 0;
	SPHERE spheres[SPH_CAPACITY];
	SEGMENT segments[SEG_CAPACITY];
	TRIANGLE triangles[TRI_CAPACITY];
	LIGHT lights[LIGHT_CAPACITY];

	COMPUTED_GEOMETRY();

	void recompute_geometry();

	// transform point based on PPC.
	bool transform(V3& v3, V3& new_v3);

	void add_sphere(SPHERE& sph);
	void add_segment(SEGMENT& seg);
	void add_triangle(TRIANGLE& tri);
	void add_mesh(MESH& mesh);
	void add_light(LIGHT& li);
};