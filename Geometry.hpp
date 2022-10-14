#pragma once

#include <vector>

#include "V3.hpp"
#include "ppc.hpp"

#define INPUT_BIN "geo.bin"
#define OUTPUT_BIN "geo.bin"
#define SEL_MESH 0 // changes index of mesh in geometry that gets read/written and also rotated.
#define SEL_LIGHT 0 // light that is moved

// LIGHTING
#define SM_TOLERANCE 20.0f // shadow-buffer tolerance
#define L_BUFFER_SIZE 500

typedef unsigned int U32;

using namespace std;

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
	V3 norm;
	vector<V3> norms;
	float phong_exp;

	TRIANGLE();
	TRIANGLE(vector<SPHERE> points);
	TRIANGLE(vector<SPHERE> points, U32 width);
};

class INTERPOLATE {
public:
	static COLOR getColor(SEGMENT& seg, V3& pos);
	static COLOR getColor(TRIANGLE& tri, V3& pos);
	static COLOR getColor(TRIANGLE& tri, COLOR& c1, COLOR& c2, COLOR& c3, V3& pos);
};

class MESH {
public:
	// NOTE: EACH TRIANGLE HAS ITS VECTORS + COLOR CONTAINED.
	vector<TRIANGLE> triangles;
	vector<vector<int>> edge_connectivity;
	bool fill;

	MESH();
	MESH(vector<TRIANGLE> tris);
	MESH(vector<TRIANGLE> tris, bool fill);
	MESH(vector<TRIANGLE> tris, bool fill, float phong_exp);

	void add_triangle(TRIANGLE tri);
	void fix_normals();

	void set_phong_exp(float exp);

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
	M33 dir_rot;
	float thresold; // cos^2 thresold angle between light_to_point and direction 
	float a; // angle of fov
	float cot_a;

	COLOR shade; // to-do

	LIGHT();
	LIGHT(V3 src, V3 direct, COLOR sh, float a);

	void check_subject(V3& point, vector<vector<float>>& l_buffer);
	bool is_subject(V3& point, vector<vector<float>>& l_buffer);
	float offset_lighting(V3& point, V3& norm, float phong_exp, vector<vector<float>>& l_buffer);
};

class GEOMETRY {
public:
	vector<SPHERE> spheres;
	vector<SEGMENT> segments;
	vector<TRIANGLE> triangles;
	vector<MESH> meshes;
	vector<LIGHT> lights;

	GEOMETRY();
	GEOMETRY(vector<GEOMETRY>& geos);

	void add_axis();
	void add_camera(PPC ppc);
};

class COMPUTED_GEOMETRY {
public:
	vector<SPHERE> spheres;
	vector<SEGMENT> segments;
	vector<SEGMENT*> segments_og;
	vector<SEGMENT> segments_mesh_og;
	vector<TRIANGLE> triangles;
	vector<TRIANGLE*> triangles_og;
	vector<LIGHT> lights;
	vector<vector<vector<float>>> l_buffer;

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