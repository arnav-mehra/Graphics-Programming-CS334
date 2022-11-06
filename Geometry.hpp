#pragma once

#include <vector>
#include <tiffio.h>
#include <algorithm>

#include "V3.hpp"
#include "ppc.hpp"

#define INPUT_BIN "geo.bin"
#define OUTPUT_BIN "geo.bin"
#define SEL_MESH 0 // changes index of mesh in geometry that gets read/written and also rotated.
#define SEL_LIGHT 0 // light that is moved

// LIGHTING
#define SM_TOLERANCE 1.1f // shadow-buffer tolerance (really sqrt of this)
#define L_BUFFER_SIZE 512
constexpr float LBS_Scalar = 0.5f * (float)(L_BUFFER_SIZE - 1);

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
	COLOR(V3 col);
	COLOR(U32 r, U32 g, U32 b);
	
	COLOR operator*(const float c);
	void operator*=(const float c);
	COLOR operator+(const COLOR& color);
	COLOR interpolate(COLOR& color, float v);
	friend ostream& operator<<(ostream& out, COLOR& c);

	U32 getR() const;
	U32 getG() const;
	U32 getB() const;
	float getBrightness() {
		return (getR() + getG() + getB()) / (3.0f * 255.0f);
	}
};

class TEXTURE {
public:
	U32 w, h;
	vector<vector<COLOR>> texture;
	vector<vector<vector<COLOR>>*> mipmap;

	TEXTURE();
	TEXTURE(char* fName);

	void initMipMap() {
		mipmap.push_back(&texture);
		while (true) {
			vector<vector<COLOR>>& prev = *(mipmap.back());
			mipmap.push_back(new vector<vector<COLOR>>());
			vector<vector<COLOR>>& next = *(mipmap.back());

			int h = prev.size() >> 1;
			int w = h == 0 ? 0 : prev[0].size() >> 1;
			next.resize(h, vector<COLOR>(w));
			for (int x = 0; x < w; x++) {
				for (int y = 0; y < h; y++) {
					COLOR c = prev[y * 2][x * 2] * 0.25f
						+ prev[y * 2 + 1][x * 2] * 0.25f
						+ prev[y * 2][x * 2 + 1] * 0.25f
						+ prev[y * 2 + 1][x * 2 + 1] * 0.25f;
					next[y][x] = c;
				}
			}

			if (w <= 1 || h <= 1) break;
		}
	}

	void transform(bool transposed, bool x_mirror, bool y_mirror);
	
	COLOR& get(int x, int y) {
		x = max(0, min(x, (int)w - 1));
		y = max(0, min(y, (int)h - 1));
		return texture[y][x];
	}

	COLOR& getMip(float x, float y) {
		COLOR sol;
		for (int i = mipmap.size() - 1; i >= 0; i--) {
			vector<vector<COLOR>>& map = *(mipmap[i]);
			float hl = map.size() - 1.0f, wl = map[0].size() - 1.0f;
			COLOR curr = map[roundf(y * hl)][roundf(x * wl)];
			if (i == mipmap.size() - 1) {
				sol = curr;
			} else {
				sol = sol * 0.8f + curr * 0.2f; // current is 4x the area, so 80/20 split
			}
		}
		return sol;
	}
};

class TEXTURE_META {
public:
	M33 t;
	TEXTURE* tx;
	float tile_factor;
	bool mirroring;

	TEXTURE_META();
	TEXTURE_META(V3 (&pts)[3], TEXTURE* tx, int tf);
	COLOR proj(V3 p);
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
	V3 points[3];
	COLOR colors[3];
	V3 norm;
	vector<V3> norms;
	float phong_exp;
	TEXTURE_META* tm;

	TRIANGLE();
	TRIANGLE(vector<V3> points);
	TRIANGLE(vector<V3> points, TEXTURE_META* tm);
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

	void add_tiling(float inc);
	void add_mirroring(bool m);

	void add_triangle(TRIANGLE tri);
	void fix_normals();

	void set_phong_exp(float exp);

	V3 get_center();
	float avg_dist_from_center();
	
	void translate(V3 tran);
	void position(V3 pos);
	void scale(float s);
	void rotate(V3 axis1, V3 axis2, float alpha);

	void setAsBox(V3 center, TEXTURE* tx, float w, float h, float l);
	void setAsSphere(V3 center, U32 hres, float radius, TEXTURE* tx);
	void setAsCylinder(V3 center, U32 res, float height, float radius);
	void setAsFloor(TEXTURE* tx);
	void setAsGlass() {
		TRIANGLE t1 = TRIANGLE({
			V3(-20.0f, -20.0f, 0),
			V3(-20.0f, 20.0f, 0),
			V3(20.0f, 20.0f, 0),			
		});
		TRIANGLE t2 = TRIANGLE({
			V3(-20.0f, -20.0f, 0),
			V3(20.0f, -20.0f, 0),
			V3(20.0f, 20.0f, 0),
		});

		t1.phong_exp = -1.0f;
		t1.norms = { V3(0,0,1), V3(0,0,1), V3(0,0,1) };
		t1.norm = V3(0, 0, 1);

		t2.phong_exp = -1.0f;
		t2.norms = { V3(0,0,1), V3(0,0,1), V3(0,0,1) };
		t2.norm = V3(0, 0, 1);

		this->add_triangle(t1);
		this->add_triangle(t2);
	}

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
	M33 dir_rot_inv;
	float thresold; // cos^2 thresold angle between light_to_point and direction 
	float a; // angle of fov
	float cot_a;

	bool proj(V3& point, V3& res);
	V3 unproj(V3& res);

	COLOR shade; // to-do

	LIGHT();
	LIGHT(V3 src, V3 direct, COLOR sh, float a);

	void check_subject_proj(V3& proj, vector<vector<float>>& l_buffer);
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
	vector<PPC> lights_ppc;
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