#include <vector>
#include <iostream>
#include <tiffio.h>
#include <algorithm>
#include <chrono>
#include <cmath>

#include "framebuffer.hpp"
#include "scene.hpp"

#define SHOW_SCENE_INTERPOLATION true
#define SHOW_SPHERICAL_INTERPOLATION false
bool IS_MIRROR = false;
bool USE_MIPMAP = false;

using namespace std;

FRAMEBUFFER::FRAMEBUFFER(int u0, int v0, U32 _w, U32 _h) : Fl_Gl_Window(u0, v0, (int) _w, (int) _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w * h];
	z_buffer.resize(w * h);
	tri_buffer.resize(w * h, -1);
}

void nextFrame(void* window) {
	auto time_start = std::chrono::system_clock::now();
	FRAMEBUFFER* fb = (FRAMEBUFFER*)window;

	fb->frame++;
	if (fb->transition1 < 1.4f) {
		scene->geometry.lights[SEL_LIGHT].source[Dim::X] -= 0.5f;
		fb->transition1 += 0.01f;
	}
	else {
		scene->geometry.lights[SEL_LIGHT].source[Dim::X] = 0.0f;
		fb->transition1 = 0.0f;
		return;
	}
	
	fb->redraw();
	auto time_end = std::chrono::system_clock::now();
	float adjustment = (1.0f / FPS) - (time_end - time_start).count() * 1e-6f;
	if (adjustment < 0.0f) adjustment = 0.0f;

	Fl::repeat_timeout(adjustment, nextFrame, window);
}

void FRAMEBUFFER::startThread() {
	transition1 = 0.0f;
	transition2 = 0.0f;
	Fl::add_timeout(1.0f / FPS, nextFrame, this);
}

void FRAMEBUFFER::applyGeometry() {
	compute.recompute_geometry();

	// reset buffers
	fill(z_buffer.begin(), z_buffer.end(), FLT_MIN);
	fill(tri_buffer.begin(), tri_buffer.end(), -1);
	compute.l_buffer.resize(compute.lights.size());
	for (vector<vector<float>>& v2 : compute.l_buffer) {
		v2.clear();
		v2.resize(L_BUFFER_SIZE, vector<float>(L_BUFFER_SIZE, FLT_MAX));
	}

	// apply shit
	for (int i = 0; i < compute.spheres.size(); i++)
		applySphere(compute.spheres[i]);
	for (int i = 0; i < compute.segments.size(); i++)
		applySegment(i);
	for (int i = 0; i < compute.triangles.size(); i++)
		applyTriangle(i);
	for (int i = 0; i < compute.triangles.size(); i++) {
		for (int j = 0; j < compute.lights.size(); j++) {
			applyTriangleLight(i, j);
		}
	}
	applyLights();
}

inline void FRAMEBUFFER::applyLights() {
	for (U32 y = 0; y < h; y++) {
		for (U32 x = 0; x < w; x++) {
			const U32 p = y * w + x;
			// filter out unset pixels
			if (z_buffer[p] == FLT_MIN || tri_buffer[p] == -1) continue;
			TRIANGLE& tri = compute.triangles[tri_buffer[p]];
			// get real_pos
			V3 pos = V3(x, y, z_buffer[p]);
			V3 real_pos = scene->ppc->unproject(pos);

			V3 norm;
			V3 dir = scene->ppc->C - real_pos;
			dir.normalize();

			float light_scalar = scene->ambient;

			if (tri.phong_exp == -1.0f) {
				norm = tri.norm * (tri.norm * dir > 0 ? 1.0f : -1.0f);
				float sin_theta = (dir ^ norm).length();
				const float n1 = 1.0f, n2 = 1.52f;
				float sin2_theta = n1 * sin_theta / n2;
				float theta2 = asin(sin2_theta);
				
				V3 axis = dir ^ norm;
				V3 new_dir = norm * -1.0f;
				new_dir.rotate(axis, -theta2);
				COLOR col = scene->bd->getColor(new_dir);
				pix[p] = col.value;
			}
			else {
				V3 a = scene->ppc->unproject(tri.points[0]);
				V3 b = scene->ppc->unproject(tri.points[1]);
				V3 c = scene->ppc->unproject(tri.points[2]);

				M33 m = M33(a, b, c);
				m.transpose();
				M33 m_inv = m.inverse();
				V3 coord = m_inv * real_pos;

				norm = tri.norms[0] * coord[0] + tri.norms[1] * coord[1] + tri.norms[2] * coord[2];
				norm.normalize();
				if (isinf(norm[Dim::X]) || isnan(-norm[Dim::X])) norm = tri.norm;

				V3 proj = norm * (dir * norm);
				V3 ref = proj * 2.0f - dir;
				ref.normalize();
				COLOR col = scene->bd->getColor(ref);
				if (IS_MIRROR) pix[p] = col.value;
				float specular = pow(fabsf(dir * ref), tri.phong_exp);
				light_scalar += col.getBrightness() * specular;
			}			

			// let there be light!
			for (int i = 0; i < compute.lights.size(); i++) {
				LIGHT& li = compute.lights[i];
				light_scalar += li.offset_lighting(real_pos, norm, tri.phong_exp, compute.l_buffer[i]);
			}
			//cout << light_scalar << '\n';
			pix[p] = (COLOR(pix[p]) * light_scalar).value;
		}
	}
}

inline void FRAMEBUFFER::applySphere(SPHERE& sphere) {
	V3& point = sphere.point;
	const U32 HALF_DOT = sphere.width >> 1;
	const U32 HALF_DOT_SQUARE = HALF_DOT * HALF_DOT;

	U32 min_x = (U32)(point[Dim::X] + 0.5f) - HALF_DOT;
	if (min_x < 0) min_x = 0;
	U32 min_y = (U32)(point[Dim::Y] + 0.5f) - HALF_DOT;
	if (min_y < 0) min_y = 0;
	U32 max_x = (U32)(point[Dim::X] - 0.5f) + HALF_DOT;
	if (max_x >= w) max_x = w - 1;
	U32 max_y = (U32)(point[Dim::Y] - 0.5f) + HALF_DOT;
	if (max_y >= h) max_y = h - 1;

	for (U32 y = min_y; y <= max_y; y++) {
		for (U32 x = min_x; x <= max_x; x++) {
			// project delta onto segment as if z = 0
			float dx = x - point[Dim::X];
			float dy = y - point[Dim::Y];
			const float dist_sq = dx * dx + dy * dy;

			// determine squared distance from point
			if (HALF_DOT_SQUARE < dist_sq) continue;

			// check if z if high enough to render over another item.
			const U32 p = y * w + x;
			if (point[Dim::Z] < z_buffer[p]) continue;
			z_buffer[p] = point[Dim::Z];

			// overwrite pixel color
			const float aa = min(HALF_DOT_SQUARE - dist_sq, 5.0f) * 0.2f; // anti-aliasing
			COLOR c = sphere.color * aa;
			pix[p] = c.value;
		}
	}
}

inline void FRAMEBUFFER::applySegment(U32 i) {
	SEGMENT& segment = compute.segments[i];
	V3& start = segment.start.point;
	V3& end = segment.end.point;
	const U32 HALF_STROKE = segment.width >> 1;
	const U32 HALF_STROKE_SQUARE = HALF_STROKE * HALF_STROKE;

	// determine box
	U32 min_x = (U32)(min(start[Dim::X], end[Dim::X]) + 0.5f) - HALF_STROKE;
	if (min_x < 0) min_x = 0;
	U32 min_y = (U32)(min(start[Dim::Y], end[Dim::Y]) + 0.5f) - HALF_STROKE;
	if (min_y < 0) min_y = 0;
	U32 max_x = (U32)(max(start[Dim::X], end[Dim::X]) - 0.5f) + HALF_STROKE;
	if (max_x >= w) max_x = w - 1;
	U32 max_y = (U32)(max(start[Dim::Y], end[Dim::Y]) - 0.5f) + HALF_STROKE;
	if (max_y >= h) max_y = h - 1;

	// compute line_vec unit vector as if z = 0
	V3 line_vec = end - start;
	const float proj_den = line_vec[Dim::X] * line_vec[Dim::X] + line_vec[Dim::Y] * line_vec[Dim::Y];
	line_vec /= sqrt(proj_den);

	// iterate over box pixels
	for (U32 y = min_y; y <= max_y; y++) {
		for (U32 x = min_x; x <= max_x; x++) {
			// project delta onto segment as if z = 0
			float dx = x - start[Dim::X];
			float dy = y - start[Dim::Y];
			const float proj_num = dx * line_vec[Dim::X] + dy * line_vec[Dim::Y];
			V3 proj = line_vec * proj_num;

			// calculate distance while ignoring z.
			dx -= proj[Dim::X];
			dy -= proj[Dim::Y];
			const float dist_sq = dx * dx + dy * dy;

			// determine squared distance from segment
			if (HALF_STROKE_SQUARE < dist_sq) continue;

			// check if z if high enough to render over another item.
			const float z_value = proj[Dim::Z] + start[Dim::Z];
			const U32 p = y * w + x;
			if (z_value < z_buffer[p]) continue;
			z_buffer[p] = z_value;

			// overwrite pixel color
			V3 pos = V3(x, y, z_value);
			V3 real_pos = scene->ppc->unproject(pos);
			const float aa = min(HALF_STROKE_SQUARE - dist_sq, 5.0f) * 0.2f; // anti-aliasing, kinda.
			COLOR gradColor = INTERPOLATE::getColor(*compute.segments_og[i], real_pos) * aa;
			
			pix[p] = gradColor.value;
		}
	}
}

inline void FRAMEBUFFER::applyTriangle(U32 i) {
	TRIANGLE& tri = compute.triangles[i];
	V3& p1 = tri.points[0];
	V3& p2 = tri.points[1];
	V3& p3 = tri.points[2];
	TEXTURE_META& tm = *(tri.tm);

	COLOR avg = (tri.colors[0] * 0.33) + (tri.colors[1] * 0.33) + (tri.colors[2] * 0.33);

	// make vectors that almost curve around the triangle.
	V3 c1 = p1 - p3;
	V3 c2 = p3 - p2;
	V3 c3 = p2 - p1;

	// z-value formula: z = (offset - n_x * x - n_y * y) / n_z
	V3 normal = c3 ^ c2; // a virtual/projected norm
	if (normal[Dim::Z] == 0.0f) return; // surface is parallel, dont render
	normal /= normal[Dim::Z]; // new formula: z = offset - n_x * x - n_y * y
	const float offset = normal * p1;

	// 0 out z-values of deltas for 2d curl
	c1[Dim::Z] = 0.0f;
	c2[Dim::Z] = 0.0f;
	c3[Dim::Z] = 0.0f;

	// determine box
	U32 min_x = (U32)(min3(p1[Dim::X], p2[Dim::X], p3[Dim::X]) + 0.5f);
	if (min_x < 0) min_x = 0;
	U32 min_y = (U32)(min3(p1[Dim::Y], p2[Dim::Y], p3[Dim::Y]) + 0.5f);
	if (min_y < 0) min_y = 0;
	U32 max_x = (U32)(max3(p1[Dim::X], p2[Dim::X], p3[Dim::X]) - 0.5f);
	if (max_x >= w) max_x = w - 1;
	U32 max_y = (U32)(max3(p1[Dim::Y], p2[Dim::Y], p3[Dim::Y]) - 0.5f);
	if (max_y >= h) max_y = h - 1;

	for (U32 y = min_y; y <= max_y; y++) {
		for (U32 x = min_x; x <= max_x; x++) {
			V3 pos = V3((float) x + 0.5, (float) y + 0.5, 0.0f);
			
			// cross product all, extract z (only non-zero value)
			V3 d1 = pos - p3;
			V3 d2 = pos - p2;
			V3 d3 = pos - p1;
			float s1 = d1.cross_z(c1);
			float s2 = d2.cross_z(c2);
			float s3 = d3.cross_z(c3);
			
			// mismatching signs = outside the triangle
			if ((s1 > 0.0f || s2 > 0.0f || s3 > 0.0f)
				&& (s1 < 0.0f || s2 < 0.0f || s3 < 0.0f)) {
				continue;
			}

			// compute z-value
			const float z_value = offset - normal[Dim::X] * ((float) x + 0.5f) - normal[Dim::Y] * ((float) y + 0.5f);
			pos[Dim::Z] = z_value;
			
			// determine if object is beneath another (shouldn't be rendered)
			const U32 p = y * w + x;
			if (z_value < z_buffer[p]) continue;
			z_buffer[p] = z_value;
			tri_buffer[p] = i;

			V3 real_pos = scene->ppc->unproject(pos);

			// compute color
			if (tri.tm)
				pix[p] = (*tri.tm).proj(real_pos).value;
			else {
				COLOR c = avg;
				if (scene->sm == 1) {
					
					c = INTERPOLATE::getColor(*compute.triangles_og[i], real_pos);
				}
				pix[p] = c.value;
			}
		}
	}
}

inline void FRAMEBUFFER::applyTriangleLight(U32 t_i, U32 l_i) {
	LIGHT& li = compute.lights[l_i];
	TRIANGLE& tri = compute.triangles[t_i];
	V3& pp1 = tri.points[0];
	V3& pp2 = tri.points[1];
	V3& pp3 = tri.points[2];

	// reproject p1/2/3 onto LIGHT
	V3 p1, p2, p3;
	if (!li.proj(scene->ppc->unproject(pp1), p1)
		|| !li.proj(scene->ppc->unproject(pp2), p2)
		|| !li.proj(scene->ppc->unproject(pp3), p3)) return;

	// make vectors that almost curve around the projected triangle.
	V3 c1 = p1 - p3;
	V3 c2 = p3 - p2;
	V3 c3 = p2 - p1;

	// z-value formula: z = (offset - n_x * x - n_y * y) / n_z
	V3 normal = c3 ^ c2; // a virtual/projected norm
	if (normal[Dim::Z] == 0.0f) return; // surface is parallel to light, dont render
	normal /= normal[Dim::Z]; // new formula: z = offset - n_x * x - n_y * y
	const float offset = normal * p1;

	// 0 out z-values of deltas for 2d curl
	c1[Dim::Z] = 0.0f;
	c2[Dim::Z] = 0.0f;
	c3[Dim::Z] = 0.0f;

	// determine box
	U32 min_x = (U32)(min3(p1[Dim::X], p2[Dim::X], p3[Dim::X]) + 0.5f);
	if (min_x < 0) min_x = 0;
	U32 min_y = (U32)(min3(p1[Dim::Y], p2[Dim::Y], p3[Dim::Y]) + 0.5f);
	if (min_y < 0) min_y = 0;
	U32 max_x = (U32)(max3(p1[Dim::X], p2[Dim::X], p3[Dim::X]) - 0.5f);
	if (max_x >= L_BUFFER_SIZE) max_x = L_BUFFER_SIZE - 1;
	U32 max_y = (U32)(max3(p1[Dim::Y], p2[Dim::Y], p3[Dim::Y]) - 0.5f);
	if (max_y >= L_BUFFER_SIZE) max_y = L_BUFFER_SIZE - 1;

	for (U32 y = min_y; y <= max_y; y++) {
		for (U32 x = min_x; x <= max_x; x++) {
			V3 pos = V3((float)x + 0.5, (float)y + 0.5, 0.0f);

			// cross product all, extract z (only non-zero value)
			V3 d1 = pos - p3;
			V3 d2 = pos - p2;
			V3 d3 = pos - p1;
			float s1 = d1.cross_z(c1);
			float s2 = d2.cross_z(c2);
			float s3 = d3.cross_z(c3);

			// mismatching signs = outside the triangle
			if ((s1 > 0.0f || s2 > 0.0f || s3 > 0.0f)
				&& (s1 < 0.0f || s2 < 0.0f || s3 < 0.0f)) {
				continue;
			}

			// compute z-value
			const float z_value = offset - normal[Dim::X] * ((float)x + 0.5f) - normal[Dim::Y] * ((float)y + 0.5f);
			pos[Dim::Z] = z_value;

			// do da lights
			li.check_subject_proj(pos, compute.l_buffer[l_i]);
		}
	}
}

void FRAMEBUFFER::draw() {
	setBackdrop();
	//SetBGR(0);
	applyGeometry();
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
}

int FRAMEBUFFER::handle(int event) {
	switch (event) {
		case FL_KEYBOARD: {
			KeyboardHandle();
			break;
		}
		case FL_MOVE: {
			int u = Fl::event_x();
			int v = Fl::event_y();
			cerr << u << " " << v << "      \r";
			break;
		}
		default: break;
	}
	return 0;
}

void FRAMEBUFFER::KeyboardHandle() {
	int key = Fl::event_key();
	switch (key) {
		case 'y': USE_MIPMAP = !USE_MIPMAP; break;
		case 't': IS_MIRROR = !IS_MIRROR; break;
		// PAN
		case FL_Left: scene->ppc->pan(0.03f); break;
		case FL_Right: scene->ppc->pan(-0.03f); break;
		// TILT
		case FL_Up: scene->ppc->tilt(-0.03f); break; 
		case FL_Down: scene->ppc->tilt(0.03f); break;
		// ROLL
		case 'e': scene->ppc->roll(-0.01f); break;
		case 'r': scene->ppc->roll(0.01f); break; 
		// TRANSLATE
		case 'w': scene->ppc->translateFB(5.0f); break;
		case 'a': scene->ppc->translateLR(-5.0f); break;
		case 's': scene->ppc->translateFB(-5.0f); break;
		case 'd': scene->ppc->translateLR(5.0f); break;
		case 'c': scene->ppc->translateUD(5.0f); break;
		case 'v': scene->ppc->translateUD(-5.0f); break;
		// ZOOM
		case '=': scene->ppc->zoom(1.1f); break;
		case '-': scene->ppc->zoom(0.9f); break;
		// TRANSLATE MESH
		case 'o': scene->geometry.meshes.back().translate(V3(0.0f, 0.0f, -10.0f)); break;
		case 'p': scene->geometry.meshes.back().translate(V3(0.0f, 0.0f, 10.0f)); break;
		// ADD/REMOVE TILING
		case 'k': scene->geometry.meshes[0].add_tiling(0.5f); break;
		case 'l': scene->geometry.meshes[0].add_tiling(-0.5f); break;
		// ADD/REMOVE MIRRORING
		case 'n': scene->geometry.meshes[0].add_mirroring(true); break;
		case 'm': scene->geometry.meshes[0].add_mirroring(false); break;
	}
	redraw();
}

void FRAMEBUFFER::SetBGR(unsigned int bgr) {
	for (U32 uv = 0; uv < w * h; uv++)
		pix[uv] = bgr;
}

void FRAMEBUFFER::setBackdrop() {
	if (scene->bd == nullptr) { // no background
		for (U32 y = 0; y < h; y++) {
			for (U32 x = 0; x < w; x++) {
				U32 px = y * w + x;
				pix[px] = COLOR(0, 0, 0).value;
			}
		}
		return;
	}
	for (U32 y = 0; y < h; y++) {
		for (U32 x = 0; x < w; x++) {
			V3 p = V3(x, y, 0.0f);
			V3 dir = scene->ppc->directional_unproject(p);
			U32 px = y * w + x;
			COLOR inter = scene->bd->getColor(dir);
			pix[px] = inter.value;
		}
	}
}

// load a tiff image to pixel buffer
void FRAMEBUFFER::LoadTiff() {
	TIFF* in = TIFFOpen(TIFF_FILE_IN, "r");

	if (in == NULL) {
		cout << TIFF_FILE_IN << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height) {
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w * h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
		cout << "failed to load " << TIFF_FILE_IN << endl;
	}
	cout << "image read in\n";

	TIFFClose(in);
}

// save as tiff image
void FRAMEBUFFER::SaveAsTiff() {
	TIFF* out = TIFFOpen(TIFF_FILE_OUT, "w");

	if (out == NULL) {
		cout << TIFF_FILE_OUT << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}

void FRAMEBUFFER::SaveAsTiff(const char* fname) {
	TIFF* out = TIFFOpen(fname, "w");

	if (out == NULL) {
		cout << TIFF_FILE_OUT << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}