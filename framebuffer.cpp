#include <vector>
#include <iostream>
#include <tiffio.h>
#include <algorithm>
#include <chrono>
#include <cmath>

#include "framebuffer.hpp"
#include "scene.hpp"

#define SCREENSPACE true

#define SHOW_SCENE_INTERPOLATION true
#define SHOW_SPHERICAL_INTERPOLATION false

using namespace std;

// load a txt file
vector<PPC> LoadPathTxt() {
	ifstream in("PATH.txt");
	PPC cam1, cam2, cam3, cam4;
	in.read((char*)&cam1, sizeof(PPC));
	in.read((char*)&cam2, sizeof(PPC));
	in.read((char*)&cam3, sizeof(PPC));
	in.read((char*)&cam4, sizeof(PPC));
	in.close();
	return { cam1, cam2, cam3, cam4 };
}

// save as txt file
void SavePathTxt() {
	ofstream out("PATH.txt");

	PPC cam1 = PPC();
	cam1.a = V3(0.896052f, 0.0f, 0.443948f);
	cam1.b = V3(0.336442f, -0.652437f, -0.679065f);
	cam1.c = V3(-206.944f, 576.624f, -303.116f);
	cam1.C = V3(-79.1066f, -278.926f, -8.77743f);

	PPC cam2 = PPC();
	cam2.a = V3(0.827187f, -0.0880683f, -0.554979f);
	cam2.b = V3(-0.377751f, -0.818327f, -0.433174f);
	cam2.c = V3(-404.614f, 539.377f, -112.066f);
	cam2.C = V3(262.227f, -176.475f, 83.9435f);

	PPC cam3 = PPC();
	cam3.a = V3(0.511406f, -0.845631f, -0.15286f);
	cam3.b = V3(-0.280801f, -0.33256f, 0.900304f);
	cam3.c = V3(-546.404f, 119.016f, -393.033f);
	cam3.C = V3(387.995f, 190.443f, -118.64f);

	PPC cam4 = PPC();
	cam4.a = V3(0.520621, -0.823524, -0.164595);
	cam4.b = V3(-0.28363, -0.346736, 0.86139);
	cam4.c = V3(-242.823, 274.127, -212.287);
	cam4.C = V3(384.325, 179.735, -112.728);

	out.write((char*)&cam1, sizeof(PPC));
	out.write((char*)&cam2, sizeof(PPC));
	out.write((char*)&cam3, sizeof(PPC));
	out.write((char*)&cam4, sizeof(PPC));
	out.close();
}

FrameBuffer::FrameBuffer(int u0, int v0, U32 _w, U32 _h) : Fl_Gl_Window(u0, v0, (int) _w, (int) _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w * h];
	z_buffer.resize(w * h);
	tri_buffer.resize(w * h, -1);
	SavePathTxt();
	transition1 = 0.0f;
	transition2 = 0.0f;
	transition3 = 0.0f;
}

void nextFrame(void* window) {
	auto time_start = std::chrono::system_clock::now();
	FrameBuffer* fb = (FrameBuffer*)window;

	if (SHOW_SCENE_INTERPOLATION) {
		fb->frame++;
		if (fb->transition1 < 2.0f) {
			scene->geometry.lights[SEL_LIGHT].source[Dim::X] -= 0.5f;
			fb->transition1 += 0.01567f;
		}
		else {
			scene->geometry.lights[SEL_LIGHT].source[Dim::X] = 30.0f;
			fb->transition1 = 0.0f;
			return;
		}
	}

	if (SHOW_SPHERICAL_INTERPOLATION) {
		if (fb->transition1 < 1.0f) {
			cout << fb->transition1;
			scene->geometry.meshes[SEL_MESH].spherical_interpolation(fb->transition1, 50.0f);
			fb->transition1 += 0.01567f;
		}
	}
	
	fb->redraw();
	auto time_end = std::chrono::system_clock::now();
	float adjustment = (1.0f / FPS) - (time_end - time_start).count() * 1e-6f;
	if (adjustment < 0.0f) adjustment = 0.0f;

	Fl::repeat_timeout(adjustment, nextFrame, window);
}

void FrameBuffer::startThread() {
	transition1 = 0.0f;
	transition2 = 0.0f;
	Fl::add_timeout(1.0f / FPS, nextFrame, this);
}

void FrameBuffer::applyGeometry() {
	compute.recompute_geometry();

	fill(z_buffer.begin(), z_buffer.end(), FLT_MIN);
	compute.l_buffer.resize(compute.lights.size());
	fill(tri_buffer.begin(), tri_buffer.end(), -1);
	for (vector<vector<float>>& v2 : compute.l_buffer) {
		v2.resize(L_BUFFER_SIZE);
		for (vector<float>& v1 : v2)
			v1.resize(L_BUFFER_SIZE, FLT_MAX);
	}

	for (int i = 0; i < compute.spheres.size(); i++)
		applySphere(compute.spheres[i]);
	for (int i = 0; i < compute.segments.size(); i++)
		applySegment(i);
	for (int i = 0; i < compute.triangles.size(); i++)
		applyTriangle(i);

	/*for (int i = 0; i < L_BUFFER_SIZE; i++) {
		for (int j = 0; j < L_BUFFER_SIZE; j++) {
			cout << compute.l_buffer[0][i][j] << ' ';
		} cout << '\n';
	}*/
	applyLights();
}

inline void FrameBuffer::applyLights() {
	if (scene->sm == 1) return;
	if (scene->sm == 2) {
		for (int i = 0; i < compute.triangles.size(); i++)
			applySM2Light(i);
		return;
	}
	if (scene->sm == 3) {
		for (U32 y = 0; y < h; y++) {
			for (U32 x = 0; x < w; x++) {
				const U32 p = y * w + x;
				// filter out unset pixels
				if (z_buffer[p] == FLT_MIN || tri_buffer[p] == -1) continue;
				TRIANGLE& tri = compute.triangles[tri_buffer[p]];
				// get real_pos
				V3 pos = V3(x, y, z_buffer[p]);
				V3 real_pos = scene->ppc->unproject(pos);
				// let there be light!
				float light_scalar = scene->ambient;
				for (int i = 0; i < compute.lights.size(); i++) {
					LIGHT& li = compute.lights[i];
					light_scalar += li.offset_lighting(real_pos, tri.norm, tri.phong_exp, compute.l_buffer[i]);
				}
				//cout << light_scalar << '\n';
				pix[p] = (COLOR(pix[p]) * light_scalar).value;
			}
		}
	}
}

inline void FrameBuffer::applySphere(SPHERE& sphere) {
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

inline void FrameBuffer::applySegment(U32 i) {
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
			COLOR gradColor = INTERPOLATE::getColor(SCREENSPACE ? segment : *compute.segments_og[i], SCREENSPACE ? pos : real_pos) * aa;
			
			pix[p] = gradColor.value;
		}
	}
}

inline void FrameBuffer::applyTriangle(U32 i) {
	TRIANGLE& tri = compute.triangles[i];
	V3& p1 = tri.points[0].point;
	V3& p2 = tri.points[1].point;
	V3& p3 = tri.points[2].point;

	COLOR avg = (tri.points[0].color * 0.33) + (tri.points[1].color * 0.33) + (tri.points[2].color * 0.33);

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
			V3 pos = V3((float) x + 0.5, (float) y  + 0.5, 0.0f);
			
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

			// update light depths (NOTE: DO BEFORE Z-BUFFER)
			V3 real_pos = scene->ppc->unproject(pos);
			for (int i = 0; i < compute.lights.size(); i++) {
				LIGHT& li = compute.lights[i];
				li.check_subject(real_pos, compute.l_buffer[i]);
			}
			
			// determine if object is beneath another (shouldn't be rendered)
			const U32 p = y * w + x;
			if (z_value < z_buffer[p]) continue;
			z_buffer[p] = z_value;
			tri_buffer[p] = i;

			// compute color
			COLOR c = avg;
			if (scene->sm == 1) {
				c = INTERPOLATE::getColor(SCREENSPACE ? tri : *compute.triangles_og[i], SCREENSPACE ? pos : real_pos);
			}
			pix[p] = c.value;
		}
	}
}

inline void FrameBuffer::applySM2Light(U32 i) {
	TRIANGLE& tri = compute.triangles[i];

	V3& p1 = tri.points[0].point;
	V3& p2 = tri.points[1].point;
	V3& p3 = tri.points[2].point;
	V3 rp1 = scene->ppc->unproject(p1);
	V3 rp2 = scene->ppc->unproject(p2);
	V3 rp3 = scene->ppc->unproject(p3);
	float light_scalar_1 = scene->ambient;
	float light_scalar_2 = scene->ambient;
	float light_scalar_3 = scene->ambient;
	

	if (tri.norms.size() == 0) {
		for (int i = 0; i < compute.lights.size(); i++) {
			LIGHT& li = compute.lights[i];
			light_scalar_1 += li.offset_lighting(rp1, tri.norm, tri.phong_exp, compute.l_buffer[i]);
			light_scalar_2 += li.offset_lighting(rp2, tri.norm, tri.phong_exp, compute.l_buffer[i]);
			light_scalar_3 += li.offset_lighting(rp3, tri.norm, tri.phong_exp, compute.l_buffer[i]);
		}
	}
	else {
		for (int i = 0; i < compute.lights.size(); i++) {
			LIGHT& li = compute.lights[i];
			light_scalar_1 += li.offset_lighting(rp1, tri.norms[0], tri.phong_exp, compute.l_buffer[i]);
			light_scalar_2 += li.offset_lighting(rp2, tri.norms[1], tri.phong_exp, compute.l_buffer[i]);
			light_scalar_3 += li.offset_lighting(rp3, tri.norms[2], tri.phong_exp, compute.l_buffer[i]);
		}
	}
	
	COLOR col1 = (tri.points[0].color * light_scalar_1).value;
	COLOR col2 = (tri.points[1].color * light_scalar_2).value;
	COLOR col3 = (tri.points[2].color * light_scalar_3).value;

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
			const U32 p = y * w + x;
			if (z_value < z_buffer[p]) continue;

			pos[Dim::Z] = z_buffer[p];
			V3 real_pos = scene->ppc->unproject(pos);
			COLOR c = INTERPOLATE::getColor(SCREENSPACE ? tri : *compute.triangles_og[i], col1, col2, col3, SCREENSPACE ? pos : real_pos);
			pix[p] = c.value;
		}
	}
}

void FrameBuffer::draw() {
	SetBGR(0);
	applyGeometry();
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
}

int FrameBuffer::handle(int event) {
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

void FrameBuffer::KeyboardHandle() {
	int key = Fl::event_key();
	switch (key) {
		// PAN
		case FL_Left: scene->ppc->pan(0.03f); break;
		case FL_Right: scene->ppc->pan(-0.03f); break;
		// TILT
		case FL_Up: scene->ppc->tilt(-0.01f); break; 
		case FL_Down: scene->ppc->tilt(0.01f); break;
		// ROLL
		case 'e': scene->ppc->roll(-0.01f); break;
		case 'r': scene->ppc->roll(0.01f); break; 
		// TRANSLATE
		case 'w': scene->ppc->translateFB(3.0f); break;
		case 'a': scene->ppc->translateLR(-3.0f); break;
		case 's': scene->ppc->translateFB(-3.0f); break;
		case 'd': scene->ppc->translateLR(3.0f); break;
		case 'c': scene->ppc->translateUD(3.0f); break;
		case 'v': scene->ppc->translateUD(-3.0f); break;
		// ZOOM
		case '=': scene->ppc->zoom(1.1f); break;
		case '-': scene->ppc->zoom(0.9f); break;
	}
	redraw();
}

void FrameBuffer::SetBGR(unsigned int bgr) {
	for (U32 uv = 0; uv < w * h; uv++)
		pix[uv] = bgr;
}

// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff() {
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
void FrameBuffer::SaveAsTiff() {
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

void FrameBuffer::SaveAsTiff(const char* fname) {
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