#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

#include "M33.hpp"
#include "Geometry.hpp"
#include "scene.hpp"

COLOR::COLOR() {}

COLOR::COLOR(U32 v) {
	value = v;
}

COLOR::COLOR(U32 r, U32 g, U32 b) {
	value = ((b << 16) | (g << 8) | r);
}

U32 COLOR::getR() const {
	return (value) & 255;
}

U32 COLOR::getG() const {
	return (value >> 8) & 255;
}

U32 COLOR::getB() const {
	return (value >> 16) & 255;
}

COLOR COLOR::operator*(const float scalar) {
	U32 r = (U32) (getR() * scalar);
	U32 g = (U32) (getG() * scalar);
	U32 b = (U32) (getB() * scalar);
	return COLOR(r, g, b);
}

void COLOR::operator*=(const float scalar) {
	(*this) = (*this) * scalar;
}

COLOR COLOR::operator+(const COLOR& color) {
	return COLOR(value + color.value);
}

COLOR COLOR::interpolate(COLOR& color, float v) {
	return (*this) * (1.0f - v) + (color) * (v);
}

COLOR INTERPOLATE::getColor(SEGMENT& seg, V3& pos) {
	float d1 = 1.0f / (pos - seg.start.point).length();
	float d2 = 1.0f / (pos - seg.end.point).length();
	float d_ = 1.0f / (d1 + d2);
	COLOR res = seg.start.color * (d_ * d1) + seg.end.color * (d_ * d2);
	return res;
}

COLOR INTERPOLATE::getColor(TRIANGLE& tri, V3& pos) {
	float d1 = 1.0f / (pos - tri.points[0].point).length();
	float d2 = 1.0f / (pos - tri.points[1].point).length();
	float d3 = 1.0f / (pos - tri.points[2].point).length();
	float d_ = 1.0f / (d1 + d2 + d3);
	COLOR res = tri.points[0].color * (d_ * d1) +
				tri.points[1].color * (d_ * d2) +
				tri.points[2].color * (d_ * d3);
	return res;
}

SPHERE::SPHERE() {}

SPHERE::SPHERE(V3 point) {
	this->point = point;
	this->width = 10;
	this->color = COLOR(255, 0, 0);
}

SPHERE::SPHERE(V3 point, COLOR color) : SPHERE(point) {
	this->color = color;
	this->width = 10;
}

SPHERE::SPHERE(V3 point, COLOR color, U32 width) : SPHERE(point, color) {
	this->width = width;
}

SEGMENT::SEGMENT() {}

SEGMENT::SEGMENT(SPHERE start, SPHERE end) {
	this->start = start;
	this->end = end;
	this->width = 5;
}

SEGMENT::SEGMENT(SPHERE start, SPHERE end, U32 width) : SEGMENT(start, end) {
	this->width = width;
}

TRIANGLE::TRIANGLE() {
	this->width = 8;
}

TRIANGLE::TRIANGLE(vector<SPHERE> points) : TRIANGLE() {
	this->points[0] = points[0];
	this->points[1] = points[1];
	this->points[2] = points[2];
}

TRIANGLE::TRIANGLE(vector<SPHERE> points, U32 width) : TRIANGLE(points) {
	this->width = width;
}

MESH::MESH() {
	fill = false;
}

MESH::MESH(vector<TRIANGLE> tris) : MESH() {
	for (TRIANGLE& tri : tris) {
		add_triangle(tri);
	}
}

MESH::MESH(vector<TRIANGLE> tris, bool fill) : MESH(tris) {
	this->fill = fill;
}

void MESH::add_triangle(TRIANGLE tri) {
	if (num_triangles >= MESH_TRI_CAPACITY) return;

	V3& p1 = tri.points[0].point;
	V3& p2 = tri.points[1].point;
	V3& p3 = tri.points[2].point;

	edge_connectivity[num_triangles][0] = -1;
	edge_connectivity[num_triangles][1] = -1;
	edge_connectivity[num_triangles][2] = -1;

	for (int i = 0; i < num_triangles; i++) {
		V3& t1 = triangles[i].points[0].point;
		V3& t2 = triangles[i].points[1].point;
		if ((p1 == t1 || p1 == t2) && (p2 == t1 || p2 == t2)) {
			edge_connectivity[num_triangles][0] = i;
			break;
		}
	}
	for (int i = 0; i < num_triangles; i++) {
		V3& t2 = triangles[i].points[1].point;
		V3& t3 = triangles[i].points[2].point;
		if ((p2 == t2 || p2 == t3) && (p3 == t2 || p3 == t3)) {
			edge_connectivity[num_triangles][1] = i;
			break;
		}
	}
	for (int i = 0; i < num_triangles; i++) {
		V3& t1 = triangles[i].points[0].point;
		V3& t3 = triangles[i].points[2].point;
		if ((p3 == t3 || p3 == t1) && (p1 == t3 || p1 == t1)) {
			edge_connectivity[num_triangles][2] = i;
			break;
		}
	}

	triangles[num_triangles++] = tri;
}

V3 MESH::get_center() {
	V3 sum = V3(0.0f, 0.0f, 0.0f);
	if (num_triangles == 0) return sum;
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			sum += p.point;
		}
	}
	V3 res = sum / ((float) num_triangles * 3.0f);
	return res;
}

void MESH::translate(V3 tran) {
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			p.point += tran;
		}
	}
}

void MESH::position(V3 pos) {
	V3 center = get_center();
	translate(pos - center);
}

void MESH::scale(float s) {
	V3 center = get_center();
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3 delta = p.point - center;
			V3 scaled = delta * s;
			p.point = scaled + center;
		}
	}
}

void MESH::rotate(V3 axis1, V3 axis2, float alpha) {
	M33 total_rotation = M33::get_rotation_matrix(axis1, axis2, alpha);
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3& v = p.point;
			v -= axis1;
			v = total_rotation * v;
			v += axis1;
		}
	}
}

float MESH::avg_dist_from_center() {
	V3 c = get_center();
	float total_dist = 0.0f;
	if (num_triangles == 0) return total_dist;
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3& v = p.point;
			V3 delta = v - c;
			total_dist += delta.length();
		}
	}
	return total_dist / (float) (num_triangles * 3);
}

void MESH::spherical_interpolation(float t, float res) {
	// break down big triangles
	for (int i = 0; i < num_triangles; i++) {
		SPHERE& s1 = triangles[i].points[0];
		SPHERE& s2 = triangles[i].points[1];
		SPHERE& s3 = triangles[i].points[2];
		float d1 = (s1.point - s2.point).length();
		float d2 = (s1.point - s3.point).length();
		float d3 = (s2.point - s3.point).length();
		float mx = max3(d1, d2, d3);
		if (mx < res) continue;
		if (mx == d1) {
			SPHERE inter = SPHERE((s1.point + s2.point) * 0.5f);
			add_triangle(TRIANGLE({ s1, inter, s3 }));
			s1 = inter; // inter, 2, 3
		}
		else if (mx == d2) {
			SPHERE inter = SPHERE((s1.point + s3.point) * 0.5f);
			add_triangle(TRIANGLE({ s1, inter, s2 }));
			s1 = inter; // inter, 2, 3
		}
		else {
			SPHERE inter = SPHERE((s2.point + s3.point) * 0.5f);
			add_triangle(TRIANGLE({ s1, inter, s3 }));
			s3 = inter; // 1, 2, inter
		}
	}
	// round triangles.
	float avg_dist = avg_dist_from_center();
	V3 c = get_center();
	for (int i = 0; i < num_triangles; i++) {
		for (SPHERE& p : triangles[i].points) {
			V3& v = p.point;
			V3 delta = v - c;
			float len = delta.length();
			float dist = (len - avg_dist) * t;
			delta /= len;
			v -= delta * dist;
		}
	}
}

void MESH::setAsBox(V3 center, float radius) {
	num_triangles = 0;

	auto A = SPHERE(V3(1.0f, 1.0f, 1.0f));
	auto B = SPHERE(V3(1.0f, -1.0f, 1.0f), COLOR(255, 255, 0));
	auto C = SPHERE(V3(-1.0f, -1.0f, 1.0f));
	auto D = SPHERE(V3(-1.0f, 1.0f, 1.0f), COLOR(255, 255, 0));

	auto E = SPHERE(V3(1.0f, 1.0f, -1.0f), COLOR(255, 255, 0));
	auto F = SPHERE(V3(1.0f, -1.0f, -1.0f));
	auto G = SPHERE(V3(-1.0f, -1.0f, -1.0f), COLOR(255, 255, 0));
	auto H = SPHERE(V3(-1.0f, 1.0f, -1.0f));

	*this = MESH({
		// ABCD
		TRIANGLE({ A, B, C }),
		TRIANGLE({ A, D, C }),
		// EFGH
		TRIANGLE({ E, F, G }),
		TRIANGLE({ E, H, G }),
		// CDHG
		TRIANGLE({ C, D, H }),
		TRIANGLE({ C, G, H }),
		// ABFE
		TRIANGLE({ A, B, F }),
		TRIANGLE({ A, E, F }),
		// DAEH
		TRIANGLE({ D, A, E }),
		TRIANGLE({ D, H, E }),
		// CBFG
		TRIANGLE({ C, B, F }),
		TRIANGLE({ C, G, F })
	});
	scale(radius);
	position(center);
}

void MESH::setAsSphere(V3 center, U32 hres, float radius) {
	num_triangles = 0;
	hres = min(hres, 20U);

	V3 vec = V3(0.0f, 0.0f, 1.0f);
	U32 vres = hres;
	float alpha = 2.0f * (float) PI / (float) hres;
	M33 rot1 = M33(Dim::Y, alpha);
	M33 rot2 = M33(Dim::X, -alpha);
	M33 rot(1);

	vector<vector<SPHERE>> vecs(vres, vector<SPHERE>(hres));
	for (U32 theta = 0; theta < vres; theta++) {
		for (U32 phi = 0; phi < hres; phi++) {
			vecs[theta][phi] = SPHERE(vec);
			vec = rot1 * vec;
			vec.normalize();
		}
		vec = rot2 * vec;
		vec.normalize();
	}

	for (U32 a = 0; a < vres; a++) {
		for (U32 b = 0; b < hres / 2; b++) {
			U32 i = (a + 1U) % hres;
			U32 j = (b + 1U) % hres;
			SPHERE& v1 = vecs[a][b];
			SPHERE& v2 = vecs[i][b];
			SPHERE& v3 = vecs[a][j];
			SPHERE& v4 = vecs[i][j];
			add_triangle(TRIANGLE({ v1, v2, v4 }));
			add_triangle(TRIANGLE({ v1, v3, v4 }));
		}
	}
	position(center);
	scale(radius);
}

void MESH::setAsCylinder(V3 center, U32 res, float height, float rad) {
	num_triangles = 0;
	res = min(res, 200U);

	float rad_inc = rad / (float) res;
	float alpha = 2.0f * (float)PI / (float)res;
	M33 rot = M33(Dim::Y, alpha);

	vector<SPHERE> upper_vecs(res);
	vector<SPHERE> lower_vecs(res);
	V3 vec = V3(rad, 0.0, 0.0);
	for (U32 theta = 0; theta < res; theta++) {
		V3 upper = vec;
		V3 lower = vec;
		upper[Dim::Y] += height * 0.5f;
		lower[Dim::Y] -= height * 0.5f;
		upper_vecs[theta] = SPHERE(upper);
		lower_vecs[theta] = SPHERE(lower);
		vec = rot * vec;
	}

	V3 upper_center = V3(0.0f, height * 0.5f, 0.0f);
	V3 lower_center = V3(0.0f, height * -0.5f, 0.0f);
	for (U32 a = 0; a < res; a++) {
		U32 i = (a + 1U) % res;
		SPHERE& v1 = lower_vecs[a];
		SPHERE& v2 = lower_vecs[i];
		SPHERE& v3 = upper_vecs[a];
		SPHERE& v4 = upper_vecs[i];
		add_triangle(TRIANGLE({ v1, v2, v4 }));
		add_triangle(TRIANGLE({ v1, v3, v4 }));
		add_triangle(TRIANGLE({ v3, v4, upper_center }));
		add_triangle(TRIANGLE({ v1, v2, lower_center }));
	}
	position(center);
}

// load a txt file
void MESH::LoadBin() {
	cout << "Loading Mesh Bin...\n";
	ifstream in(INPUT_BIN);
	in.read((char*)this, sizeof(MESH));
	in.close();
}

void MESH::Load334Bin() {
	ifstream ifs(INPUT_BIN, ios::binary);

	int vertsN;
	ifs.read((char*) &vertsN, sizeof(int));

	char yn;
	ifs.read(&yn, 1);
	bool cols = yn == 'y';
	ifs.read(&yn, 1);
	bool norms = yn == 'y';
	ifs.read(&yn, 1);
	bool tcs = yn == 'y';

	V3* verts = new V3[vertsN];
	ifs.read((char*)verts, vertsN * 3 * sizeof(float));
	
	if (cols) {
		V3* cols = new V3[vertsN];
		ifs.read((char*)cols, vertsN * 3 * sizeof(float));
	}
	if (norms) {
		V3* normals = new V3[vertsN];
		ifs.read((char*)normals, vertsN * 3 * sizeof(float));
	}
	if (tcs) {
		float* tcs = new float[vertsN * 2];
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float));
	}

	ifs.read((char*) &num_triangles, sizeof(int));
	float* tris = new float[num_triangles * 3];
	ifs.read((char*) tris, num_triangles * 3 * sizeof(float)); // read tiangles

	ifs.close();
}

// save as txt file
void MESH::SaveAsBin() {
	cout << "Saving Mesh Bin...\n";
	ofstream out(OUTPUT_BIN);
	out.write((char*)this, sizeof(MESH));
	out.close();
}

LIGHT::LIGHT() {}

LIGHT::LIGHT(V3 src, V3 direct, COLOR sh, float _a) {
	source = src;
	direction = direct;
	direction.normalize();
	shade = sh;
	a = _a;

	float cos_a = cos(DEG_TO_RAD(a)); // = direct.mag / edge
	thresold = cos_a * cos_a; // (direction * edge)^2 / (|direction||edge|)^2
}

bool LIGHT::is_subject(V3& point) {
	// angle check
	V3 delta = point - source;
	float dot = delta * direction;
	if (dot < 0.0f) return false; // angle delta + light dir is acute
	// norm check
	float cos_sq = (dot * dot) / (delta * delta);
	return cos_sq >= thresold; // falls within the fov
}

float LIGHT::offset_lighting(V3& point, V3& norm, float phong_exp) {
	if (!is_subject(point)) return 0.0f; // not hit by light -> ambient lighting
	else return 1.0f;
	// rotate point_to_light PI radian about normal using projection
	//V3 point_to_light = source - point;
	//float dot = norm * point_to_light;
	//float proj = (dot) / (norm * norm);
	//V3 reflected_light = norm * (proj * 2.0f) - point_to_light;
	//// use phong's method
	//V3 eye_vec = point - scene->ppc->C;
	//float k_diffuse = max(dot, 0.0f);
	//float k_specular = pow(eye_vec * reflected_light, phong_exp);
	//if (k_diffuse > 1.0e6f) {
	//	cout << ":{";
	//}
	//return K_AMBIENT + (1.0f - K_AMBIENT) * k_diffuse + k_specular;
}

GEOMETRY::GEOMETRY() {}

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
		for (int i = 0; i < geo.num_meshes; i++) {
			add_mesh(geo.meshes[i]);
		}
		for (int i = 0; i < geo.num_lights; i++) {
			add_light(geo.lights[i]);
		}
	}
}

GEOMETRY::GEOMETRY(vector<SPHERE> spheres,
				   vector<SEGMENT> segments,
				   vector<TRIANGLE> triangles,
				   vector<MESH> meshes,
				   vector<LIGHT> lights) {
	for (SPHERE& p : spheres) add_sphere(p);
	for (SEGMENT& s : segments) add_segment(s);
	for (TRIANGLE& t : triangles) add_triangle(t);
	for (MESH& m : meshes) add_mesh(m);
	for (LIGHT& l : lights) add_light(l);
}

void GEOMETRY::add_axis() {
	add_segment(
		SEGMENT(
			SPHERE(V3(-200, 0, 0)),
			SPHERE(V3(200, 0, 0))
		)
	);
	add_segment(
		SEGMENT(
			SPHERE(V3(0, -200, 0)),
			SPHERE(V3(0, 200, 0))
		)
	);
	add_segment(
		SEGMENT(
			SPHERE(V3(0, 0, -200)),
			SPHERE(V3(0, 0, 200))
		)
	);
}

void GEOMETRY::add_sphere(SPHERE sph) {
	if (num_spheres < SPH_CAPACITY)
		spheres[num_spheres++] = sph;
}

void GEOMETRY::add_segment(SEGMENT seg) {
	if (num_segments < SEG_CAPACITY)
		segments[num_segments++] = seg;
}

void GEOMETRY::add_triangle(TRIANGLE tri) {
	if (num_triangles < TRI_CAPACITY)
		triangles[num_triangles++] = tri;
}

void GEOMETRY::add_mesh(MESH mesh) {
	if (num_meshes < MESH_CAPACITY)
		meshes[num_meshes++] = mesh;
}

void GEOMETRY::add_light(LIGHT li) {
	if (num_lights < MESH_CAPACITY)
		lights[num_lights++] = li;
}

COMPUTED_GEOMETRY::COMPUTED_GEOMETRY() {
	recompute_geometry();
}

// rotate + copy geometry
void COMPUTED_GEOMETRY::recompute_geometry() {
	num_segments = 0;
	num_spheres = 0;
	num_triangles = 0;
	num_lights = 0;
	GEOMETRY& geometry = scene->geometry;

	for (int i = 0; i < geometry.num_segments; i++)
		add_segment(geometry.segments[i]);
	for (int i = 0; i < geometry.num_triangles; i++)
		add_triangle(geometry.triangles[i]);
	for (int i = 0; i < geometry.num_spheres; i++)
		add_sphere(geometry.spheres[i]);
	for (int i = 0; i < geometry.num_meshes; i++)
		add_mesh(geometry.meshes[i]);
	for (int i = 0; i < geometry.num_lights; i++)
		add_light(geometry.lights[i]);
}

// rotate + translate each V3 depending on perspective + origin. 
bool COMPUTED_GEOMETRY::transform(V3& v3, V3& new_v3) {
	if (!scene->ppc->project(v3, new_v3)) {
		return false;
	}
	return true;
}

void COMPUTED_GEOMETRY::add_sphere(SPHERE& sph) {
	SPHERE& new_sph = spheres[num_spheres];
	if (transform(sph.point, new_sph.point)) {
		new_sph.color = sph.color;
		new_sph.width = sph.width;
		num_spheres++; // save new sphere
	}
}

void COMPUTED_GEOMETRY::add_segment(SEGMENT& seg) {
	SEGMENT& new_seg = segments[num_segments];
	if (transform(seg.start.point, new_seg.start.point) &&
		transform(seg.end.point, new_seg.end.point)) {

		new_seg.start.color = seg.start.color;
		new_seg.start.width = seg.start.width;
		new_seg.end.color = seg.end.color;
		new_seg.end.width = seg.end.width;
		new_seg.width = seg.width;
		num_segments++; // save new segment
	}
}

void COMPUTED_GEOMETRY::add_triangle(TRIANGLE& tri) {
	TRIANGLE& new_tri = triangles[num_triangles];
	if (transform(tri.points[0].point, new_tri.points[0].point) &&
		transform(tri.points[1].point, new_tri.points[1].point) &&
		transform(tri.points[2].point, new_tri.points[2].point)) {

		new_tri.points[0].color = tri.points[0].color;
		new_tri.points[0].width = tri.points[0].width;
		new_tri.points[1].color = tri.points[1].color;
		new_tri.points[1].width = tri.points[1].width;
		new_tri.points[2].color = tri.points[2].color;
		new_tri.points[2].width = tri.points[2].width;
		num_triangles++; // save new triangle
	}
}

void COMPUTED_GEOMETRY::add_mesh(MESH& mesh) {
	if (mesh.fill) {
		for (int i = 0; i < mesh.num_triangles; i++) {
			add_triangle(mesh.triangles[i]);
		}
		return;
	}
	
	for (int i = 0; i < mesh.num_triangles; i++) {
		TRIANGLE& tri = mesh.triangles[i];
		SPHERE p1, p2, p3;
			
		if (transform(tri.points[0].point, p1.point) &&
			transform(tri.points[1].point, p2.point) &&
			transform(tri.points[2].point, p3.point)) {

			p1.color = tri.points[0].color;
			p2.color = tri.points[1].color;
			p3.color = tri.points[2].color;
				
			if (mesh.edge_connectivity[i][0] == -1)
				segments[num_segments++] = SEGMENT(p1, p2);
			if (mesh.edge_connectivity[i][1] == -1)
				segments[num_segments++] = SEGMENT(p2, p3);
			if (mesh.edge_connectivity[i][2] == -1)
				segments[num_segments++] = SEGMENT(p3, p1);
		}
	}
}

void COMPUTED_GEOMETRY::add_light(LIGHT& li) {
	V3 new_src;
	V3 dir_end;
	if (transform(li.source, new_src) &&
		transform(li.direction, dir_end)) {

		add_sphere(SPHERE(li.source, COLOR(255, 255, 255), 10));
		add_segment(
			SEGMENT(
				SPHERE(li.source, COLOR(255, 255, 255)),
				SPHERE(li.source + li.direction * 200.0f, COLOR(255, 255, 255)),
				5
			)
		);
		lights[num_lights++] = LIGHT(new_src, dir_end - new_src, li.shade, li.a);
	}
}