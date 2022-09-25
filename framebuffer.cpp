#include <vector>
#include <iostream>
#include <tiffio.h>
#include <algorithm>
#include <chrono>
#include <cmath>

#include "framebuffer.hpp"
#include "_Geometry.hpp"

#define max3(x, y, z) (max(max((x), (y)), (z)))
#define min3(x, y, z) (min(min((x), (y)), (z)))
using namespace std;

FrameBuffer::FrameBuffer(int u0, int v0, U32 _w, U32 _h) : Fl_Gl_Window(u0, v0, (int) _w, (int) _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w * h];

	cam1 = PPC();
	cam1.hfovd = 1.0472f;
	cam1.a = V3(0.896052f, 0.0f, 0.443948f);
	cam1.b = V3(0.336442f, -0.652437f, -0.679065f);
	cam1.c = V3(-206.944f, 576.624f, -303.116f);
	cam1.C = V3(-79.1066f, -278.926f, -8.77743f);

	cam2 = PPC();
	cam2.a = V3(0.827187f, -0.0880683f, -0.554979f);
	cam2.b = V3(-0.377751f, -0.818327f, -0.433174f);
	cam2.c = V3(-404.614f, 539.377f, -112.066f);
	cam2.C = V3(262.227f, -176.475f, 83.9435f);
	cam2.hfovd = 1.0472f;

	cam3 = PPC();
	cam3.a = V3(0.511406f, -0.845631f, -0.15286f);
	cam3.b = V3(-0.280801f, -0.33256f, 0.900304f);
	cam3.c = V3(-546.404f, 119.016f, -393.033f);
	cam3.C = V3(387.995f, 190.443f, -118.64f);
	cam3.hfovd = 1.0472f;

	transition1 = 0.0f;
	transition2 = 0.0f;
}

void nextFrame(void* window) {
	auto time_start = std::chrono::system_clock::now();
	FrameBuffer* fb = (FrameBuffer*)window;

	if (false) {
		if (fb->transition1 < 1.0f) {
			cout << "T1: " << fb->transition1 << '\n';
			scene->ppc->interpolate(fb->cam1, fb->cam2, fb->transition1);
			fb->transition1 += 0.015f;
		}
		else if (fb->transition2 < 1.0f) {
			cout << "T2: " << fb->transition2 << '\n';
			scene->ppc->interpolate(fb->cam2, fb->cam3, fb->transition2);
			fb->transition2 += 0.015f;
		}
		else {
			return;
		}
	}
	else {
		if (fb->transition1 < 1.0f) {
			scene->geometry.meshes[0].sphericalInterpolation(fb->transition1);
			fb->transition1 += 0.015f;
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
	compute = COMPUTED_GEOMETRY();
	vector<float> z_index(w * h, FLT_MAX);

	for (int i = 0; i < compute.num_segments; i++) {
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
		const float line_vec_normalizer = 1.0f / sqrt(proj_den);
		line_vec *= line_vec_normalizer;

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
				if (z_value > z_index[p]) continue;
				z_index[p] = z_value;

				// overwrite pixel color
				const float d = min(HALF_STROKE_SQUARE - dist_sq, 5.0f);
				const float gradientShare = proj.length() * line_vec_normalizer;
				// cout << gradientShare << '\n';
				COLOR gradientColor = segment.start.color.interpolate(segment.end.color, gradientShare);
				gradientColor *= (0.2f * d);
				pix[p] = gradientColor.value;
			}
		}
	}

	for (int i = 0; i < compute.num_spheres; i++) {
		SPHERE& sphere = compute.spheres[i];
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
				if (point[Dim::Z] > z_index[p]) continue;
				z_index[p] = point[Dim::Z];

				// overwrite pixel color
				const float d = min(HALF_DOT_SQUARE - dist_sq, 5.0f);
				COLOR c = sphere.color * (0.2f * d);
				pix[p] = c.value;
			}
		}
	}

	// cool method inspired by vector field curl
	for (int i = 0; i < compute.num_triangles; i++) {
		TRIANGLE& tri = compute.triangles[i];
		V3 p1 = tri.points[0].point; p1[Dim::Z] = 0.0f;
		V3 p2 = tri.points[1].point; p2[Dim::Z] = 0.0f;
		V3 p3 = tri.points[2].point; p3[Dim::Z] = 0.0f;
		// make vectors that almost curve around the triangle.
		V3 c1 = p1 - p3;
		V3 c2 = p3 - p2;
		V3 c3 = p2 - p1;

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
				V3 pos = V3((float)x, (float)y, 0.0f);
				V3 d1 = pos - p3;
				V3 d2 = pos - p2;
				V3 d3 = pos - p1;
				// cross product all, extract z (only non-zero value)
				V3 r1 = d1 ^ c1;
				V3 r2 = d2 ^ c2;
				V3 r3 = d3 ^ c3;
				float cross1 = r1[Dim::Z];
				float cross2 = r2[Dim::Z];
				float cross3 = r3[Dim::Z];
				// matching signs = inside the triangle
				if ((cross1 < 0 && cross2 < 0 && cross3 < 0)
					|| (cross1 > 0 && cross2 > 0 && cross3 > 0)) {
					const U32 p = y * w + x;
					if (0 > z_index[p]) continue;
					z_index[p] = 0;
					pix[p] = tri.points[0].color.value;
				}
			}
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
		case FL_Left: {
			M33 rot = M33(Dim::Y, 0.01f);
			scene->ppc->rotate(rot);
			break;
		}
		case FL_Right: {
			M33 rot = M33(Dim::Y, -0.01f);
			scene->ppc->rotate(rot);
			break;
		}
		// ROLL
		case FL_Up: {
			M33 rot = M33(Dim::X, -0.01f);
			scene->ppc->rotate(rot);
			break;
		}
		case FL_Down: {
			M33 rot = M33(Dim::X, 0.01f);
			scene->ppc->rotate(rot);
			break;
		}
		// TILT
		case 'e': {
			M33 rot = M33(Dim::Z, 0.1f);
			scene->ppc->rotate(rot);
			break;
		}
		case 'r': {
			M33 rot = M33(Dim::Z, -0.1f);
			scene->ppc->rotate(rot);
			break;
		}
		// TRANSLATE
		case 'w': {
			scene->ppc->translateFB(1.0f);
			break;
		}
		case 'a': {
			scene->ppc->translateLR(-1.0f);
			break;
		}
		case 's': {
			scene->ppc->translateFB(-1.0f);
			break;
		}
		case 'd': {
			scene->ppc->translateLR(1.0f);
			break;
		}
		case 'c': {
			scene->ppc->translateUD(1.0f);
			break;
		}
		case 'v': {
			scene->ppc->translateUD(-1.0f);
			break;
		}
		// ZOOM
		case '=': {
			scene->ppc->zoom(1.1f);
			break;
		}
		case '-': {
			scene->ppc->zoom(0.9f);
			break;
		}
	}
	redraw();
}

void FrameBuffer::SetBGR(unsigned int bgr) {
	for (int uv = 0; uv < w * h; uv++)
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